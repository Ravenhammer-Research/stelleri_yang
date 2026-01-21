#include "IetfRouting.hpp"
#include "Exceptions.hpp"
#include <libyang/libyang.h>
#include "IetfInterfaces.hpp"

using namespace yang;

static void check_ly_err(const YangContext &ctx, LY_ERR err) {
  if (err != LY_SUCCESS)
    throw YangDataError(ctx);
}

struct lyd_node *IetfRouting::serialize(const YangContext &ctx) const {
  struct ly_ctx *c = ctx.raw();

  struct lyd_node *root = nullptr;
  check_ly_err(ctx, lyd_new_path(nullptr, c, "/ietf-routing:routing", NULL, 0, &root));

  // router-id
  if (routing_.router_id.has_value()) {
    struct lyd_node *tmp = nullptr;
    check_ly_err(ctx, lyd_new_path(root, c, "router-id", routing_.router_id->c_str(), 0, &tmp));
  }

  // interfaces leaf-list
  for (const auto &ifname : routing_.interfaces) {
    struct lyd_node *tmp = nullptr;
    check_ly_err(ctx, lyd_new_path(root, c, "interfaces/interface", ifname.c_str(), 0, &tmp));
  }

  // control-plane-protocol entries (type + name + description)
  for (const auto &cpp : routing_.control_plane_protocols) {
    std::string pred = "control-plane-protocols/control-plane-protocol[type='" + cpp.type + "'][name='" + cpp.name + "']";
    struct lyd_node *tmp = nullptr;
    check_ly_err(ctx, lyd_new_path(root, c, (pred + "/type").c_str(), cpp.type.c_str(), 0, &tmp));
    check_ly_err(ctx, lyd_new_path(root, c, (pred + "/name").c_str(), cpp.name.c_str(), 0, &tmp));
    if (cpp.description.has_value())
      check_ly_err(ctx, lyd_new_path(root, c, (pred + "/description").c_str(), cpp.description->c_str(), 0, &tmp));

    // static-routes: create minimal entries
    for (const auto &r : cpp.static_routes) {
      if (r.route_preference.has_value()) {
        struct lyd_node *tmp2 = nullptr;
        check_ly_err(ctx, lyd_new_path(root, c, (pred + "/static-routes/route/route-preference").c_str(), std::to_string(*r.route_preference).c_str(), 0, &tmp2));
      }
    }
  }

  // ribs
  for (const auto &rib : routing_.ribs) {
    std::string pred = "ribs/rib[name='" + rib.name + "']";
    struct lyd_node *tmp = nullptr;
    check_ly_err(ctx, lyd_new_path(root, c, (pred + "/name").c_str(), rib.name.c_str(), 0, &tmp));
    check_ly_err(ctx, lyd_new_path(root, c, (pred + "/address-family").c_str(), rib.address_family.c_str(), 0, &tmp));
    if (rib.description.has_value())
      check_ly_err(ctx, lyd_new_path(root, c, (pred + "/description").c_str(), rib.description->c_str(), 0, &tmp));
    for (const auto &r : rib.routes) {
      if (r.route_preference.has_value()) {
        struct lyd_node *tmp2 = nullptr;
        check_ly_err(ctx, lyd_new_path(root, c, (pred + "/routes/route/route-preference").c_str(), std::to_string(*r.route_preference).c_str(), 0, &tmp2));
      }
    }
  }

  return root;
}

static struct lyd_node *find_child_by_name(struct lyd_node *parent, const char *name) {
  if (!parent)
    return nullptr;
  for (struct lyd_node *c = lyd_child(parent); c; c = c->next) {
    if (c->schema && c->schema->name && strcmp(c->schema->name, name) == 0)
      return c;
  }
  return nullptr;
}

static const char *get_node_value(struct lyd_node *n) {
  if (!n)
    return nullptr;
  return lyd_get_value(n);
}

std::unique_ptr<IetfRouting> IetfRouting::deserialize(const YangContext &ctx, struct lyd_node *tree) {
  if (!tree)
    throw YangDataError(ctx);

  auto model = std::make_unique<IetfRouting>();

  const char *v = nullptr;

  // 1) If a top-level /ietf-interfaces:interfaces exists, parse it first
  //    so the routing model can obtain full interface objects up-front.
  struct lyd_node *ifs_tree = nullptr;
  if (lyd_find_path(tree, "/ietf-interfaces:interfaces", 0, &ifs_tree) == LY_SUCCESS && ifs_tree != nullptr) {
    auto ifs_model = IetfInterfaces::deserialize(ctx, ifs_tree);
    if (ifs_model) {
      for (const auto &iface : ifs_model->getInterfaces()) {
        model->mutableRouting().interfaces_info.push_back(iface);
        model->mutableRouting().interfaces.push_back(iface.name);
      }
    }
  }

  // 2) Find the routing node (either the tree itself or a child)
  struct lyd_node *rt = nullptr;
  if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "routing") == 0) {
    rt = tree;
  } else {
    if (lyd_find_path(tree, "/ietf-routing:routing", 0, &rt) != LY_SUCCESS)
      rt = nullptr;
  }
  if (!rt)
    throw YangDataError(ctx);

  // router-id
  struct lyd_node *rid = find_child_by_name(rt, "router-id");
  if (rid && (v = get_node_value(rid)))
    model->mutableRouting().router_id = std::string(v);

  // routing-local interfaces leaf-list: append any names that are not
  // already present from the parsed top-level interfaces above.
  struct lyd_node *ifs = find_child_by_name(rt, "interfaces");
  if (ifs) {
    for (struct lyd_node *ch = lyd_child(ifs); ch; ch = ch->next) {
      if (!ch->schema || !ch->schema->name)
        continue;
      if (strcmp(ch->schema->name, "interface") == 0) {
        const char *val = get_node_value(ch);
        if (val) {
          bool found = false;
          for (const auto &n : model->mutableRouting().interfaces) {
            if (n == val) { found = true; break; }
          }
          if (!found)
            model->mutableRouting().interfaces.push_back(std::string(val));
        }
      }
    }
  }

  // control-plane-protocols
  struct lyd_node *cpps = find_child_by_name(rt, "control-plane-protocols");
  if (cpps) {
    for (struct lyd_node *entry = lyd_child(cpps); entry; entry = entry->next) {
      if (!entry->schema || !entry->schema->name)
        continue;
      if (strcmp(entry->schema->name, "control-plane-protocol") != 0)
        continue;
      ControlPlaneProtocol cp;
      struct lyd_node *t = find_child_by_name(entry, "type");
      if (t && (v = get_node_value(t))) {
        std::string sv(v);
        auto pos = sv.find(':');
        if (pos != std::string::npos)
          sv = sv.substr(pos + 1);
        cp.type = sv;
      }
      struct lyd_node *nnode = find_child_by_name(entry, "name");
      if (nnode && (v = get_node_value(nnode)))
        cp.name = v;
      struct lyd_node *d = find_child_by_name(entry, "description");
      if (d && (v = get_node_value(d)))
        cp.description = std::string(v);

      // static-routes (parse route-preference)
      struct lyd_node *sr = find_child_by_name(entry, "static-routes");
      if (sr) {
        for (struct lyd_node *r = lyd_child(sr); r; r = r->next) {
          if (!r->schema || !r->schema->name)
            continue;
          if (strcmp(r->schema->name, "route") != 0)
            continue;
          Route route;
          struct lyd_node *rp = find_child_by_name(r, "route-preference");
          if (rp && (v = get_node_value(rp)))
            route.route_preference = static_cast<uint32_t>(std::stoul(v));
          cp.static_routes.push_back(route);
        }
      }

      model->mutableRouting().control_plane_protocols.push_back(cp);
    }
  }

  // ribs
  struct lyd_node *ribs = find_child_by_name(rt, "ribs");
  if (ribs) {
    for (struct lyd_node *entry = lyd_child(ribs); entry; entry = entry->next) {
      if (!entry->schema || !entry->schema->name)
        continue;
      if (strcmp(entry->schema->name, "rib") != 0)
        continue;
      Rib rib;
      struct lyd_node *nnode = find_child_by_name(entry, "name");
      if (nnode && (v = get_node_value(nnode)))
        rib.name = v;
      struct lyd_node *af = find_child_by_name(entry, "address-family");
      if (af && (v = get_node_value(af))) {
        std::string sv(v);
        auto pos = sv.find(':');
        if (pos != std::string::npos)
          sv = sv.substr(pos + 1);
        rib.address_family = sv;
      }
      struct lyd_node *desc = find_child_by_name(entry, "description");
      if (desc && (v = get_node_value(desc)))
        rib.description = std::string(v);

      struct lyd_node *routes = find_child_by_name(entry, "routes");
      if (routes) {
        for (struct lyd_node *r = lyd_child(routes); r; r = r->next) {
          if (!r->schema || !r->schema->name)
            continue;
          if (strcmp(r->schema->name, "route") != 0)
            continue;
          Route route;
          struct lyd_node *rp = find_child_by_name(r, "route-preference");
          if (rp && (v = get_node_value(rp)))
            route.route_preference = static_cast<uint32_t>(std::stoul(v));
          rib.routes.push_back(route);
        }
      }

      model->mutableRouting().ribs.push_back(rib);
    }
  }

  return model;
}
