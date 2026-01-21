// IetfInterfaces.cpp
// Cleaned-up serialization/deserialization helpers for the
// `ietf-interfaces` YANG model.

#include "IetfInterfaces.hpp"
#include "Exceptions.hpp"

#include <format>
#include <libyang/libyang.h>
#include <regex>
#include <string>

using namespace yang;

// Small helper: find the first immediate child node with the given schema
// name. Returns nullptr if not found.
static struct lyd_node *find_child_node(struct lyd_node *parent,
                                        const char *name) {
  if (!parent || !name)
    return nullptr;
  for (struct lyd_node *c = lyd_child(parent); c; c = c->next) {
    if (c->schema && c->schema->name && strcmp(c->schema->name, name) == 0)
      return c;
  }
  return nullptr;
}

static const char *node_value(struct lyd_node *n) {
  return n ? lyd_get_value(n) : nullptr;
}

struct lyd_node *IetfInterfaces::serialize(const YangContext &ctx) const {
  struct ly_ctx *c = ctx.raw();
  struct lyd_node *root = nullptr;

  if (lyd_new_path(nullptr, c, "/ietf-interfaces:interfaces", nullptr, 0,
                   &root) != LY_SUCCESS ||
      !root) {
    throw YangDataError(ctx);
  }

  for (const auto &it : ifs_) {
    const std::string &name = it.name;

    // Required leaf: name
    struct lyd_node *tmp = nullptr;
    lyd_new_path(root, c,
                 std::format("interface[name='{}']/name", name).c_str(),
                 name.c_str(), 0, &tmp);

    if (it.description.has_value()) {
      lyd_new_path(
          root, c,
          std::format("interface[name='{}']/description", name).c_str(),
          it.description->c_str(), 0, &tmp);
    }

    if (!it.enabled) {
      lyd_new_path(root, c,
                   std::format("interface[name='{}']/enabled", name).c_str(),
                   "false", 0, &tmp);
    }

    if (it.type.has_value()) {
      const std::string t =
          std::format("ianaift:{}", yang::ianaIfTypeToString(*it.type));
      lyd_new_path(root, c,
                   std::format("interface[name='{}']/type", name).c_str(),
                   t.c_str(), 0, &tmp);
    }

    // IPv4 addresses and container-level mtu
    if (it.ipv4.has_value()) {
      for (const auto &addr : it.ipv4->address) {
        std::string ipstr = addr.address;
        std::string ip_only;
        std::string prefix;
        auto slash = ipstr.find('/');
        if (slash != std::string::npos) {
          ip_only = ipstr.substr(0, slash);
          prefix = ipstr.substr(slash + 1);
        } else {
          ip_only = ipstr;
        }

        const std::string base = std::format(
            "interface[name='{}']/ip:ipv4/ip:address[ip='{}']", name, ip_only);
        lyd_new_path(root, c, (base + "/ip:ip").c_str(), ip_only.c_str(), 0,
                     &tmp);
        if (!prefix.empty()) {
          lyd_new_path(root, c, (base + "/ip:prefix-length").c_str(),
                       prefix.c_str(), 0, &tmp);
        }
      }
      if (it.ipv4->mtu.has_value()) {
        lyd_new_path(
            root, c,
            std::format("interface[name='{}']/ip:ipv4/mtu", name).c_str(),
            std::format("{}", *it.ipv4->mtu).c_str(), 0, &tmp);
      }
    }

    // IPv6 addresses and container-level mtu
    if (it.ipv6.has_value()) {
      for (const auto &addr : it.ipv6->address) {
        std::string ipstr = addr.address;
        std::string ip_only;
        std::string prefix;
        auto slash = ipstr.find('/');
        if (slash != std::string::npos) {
          ip_only = ipstr.substr(0, slash);
          prefix = ipstr.substr(slash + 1);
        } else {
          ip_only = ipstr;
        }

        const std::string base = std::format(
            "interface[name='{}']/ip:ipv6/ip:address[ip='{}']", name, ip_only);
        lyd_new_path(root, c, (base + "/ip:ip").c_str(), ip_only.c_str(), 0,
                     &tmp);
        if (!prefix.empty()) {
          lyd_new_path(root, c, (base + "/ip:prefix-length").c_str(),
                       prefix.c_str(), 0, &tmp);
        }
      }
      if (it.ipv6->mtu.has_value()) {
        lyd_new_path(
            root, c,
            std::format("interface[name='{}']/ip:ipv6/mtu", name).c_str(),
            std::format("{}", *it.ipv6->mtu).c_str(), 0, &tmp);
      }
    }
  }
  return root;
}

std::unique_ptr<IetfInterfaces>
IetfInterfaces::deserialize(const YangContext &ctx, struct lyd_node *tree) {
  if (!tree)
    throw YangDataError(ctx);

  auto model = std::make_unique<IetfInterfaces>();

  struct lyd_node *ifs = nullptr;
  if (tree->schema && tree->schema->name &&
      strcmp(tree->schema->name, "interfaces") == 0) {
    ifs = tree;
  } else {
    if (lyd_find_path(tree, "/ietf-interfaces:interfaces", 0, &ifs) !=
        LY_SUCCESS) {
      ifs = nullptr;
    }
  }
  if (!ifs)
    throw YangDataError(ctx);

  const char *v = nullptr;
  static const std::regex ns_prefix(R"(^[^:]+:)");

  for (struct lyd_node *ch = lyd_child(ifs); ch; ch = ch->next) {
    if (!ch->schema || !ch->schema->name)
      continue;
    if (strcmp(ch->schema->name, "interface") != 0)
      continue;

    IetfInterfaces::IetfInterface itf;

    struct lyd_node *n = find_child_node(ch, "name");
    if (n && (v = node_value(n)))
      itf.name = v;
    if (itf.name.empty())
      throw YangDataError(ctx);

    n = find_child_node(ch, "description");
    if (n && (v = node_value(n)))
      itf.description = std::string(v);

    n = find_child_node(ch, "type");
    if (n && (v = node_value(n))) {
      std::string sv(v);
      sv = std::regex_replace(sv, ns_prefix, "");
      itf.type = yang::ianaIfTypeFromString(sv);
    }

    n = find_child_node(ch, "enabled");
    if (n && (v = node_value(n)))
      itf.enabled = !(strcmp(v, "false") == 0 || strcmp(v, "0") == 0);

    n = find_child_node(ch, "admin-status");
    if (n && (v = node_value(n)))
      itf.admin_status = std::string(v);

    n = find_child_node(ch, "oper-status");
    if (n && (v = node_value(n)))
      itf.oper_status = std::string(v);

    // statistics/discontinuity-time
    n = find_child_node(ch, "statistics");
    if (n) {
      struct lyd_node *stat = find_child_node(n, "discontinuity-time");
      if (stat && (v = node_value(stat))) {
        IetfInterfaces::IetfInterfaceStatistics s;
        std::string dt(v);
        // Normalize libyang's "+00:00" to 'Z' for UTC
        if (dt.size() >= 6 && dt.compare(dt.size() - 6, 6, "+00:00") == 0)
          dt = dt.substr(0, dt.size() - 6) + "Z";
        s.discontinuity_time = dt;
        itf.statistics = s;
      }
    }

    // ipv4 augmentation
    n = find_child_node(ch, "ipv4");
    if (n) {
      IetfInterfaces::IetfIpv4 ip4;
      struct lyd_node *mtu_leaf = find_child_node(n, "mtu");
      if (mtu_leaf && (v = node_value(mtu_leaf)))
        ip4.mtu = static_cast<uint32_t>(std::stoul(v));

      for (struct lyd_node *addr = lyd_child(n); addr; addr = addr->next) {
        if (!addr->schema || !addr->schema->name)
          continue;
        if (strcmp(addr->schema->name, "address") != 0)
          continue;
        struct lyd_node *ip_leaf = find_child_node(addr, "ip");
        if (ip_leaf && (v = node_value(ip_leaf))) {
          IetfInterfaces::IetfIpv4::Address a;
          a.address = std::string(v);
          struct lyd_node *pl = find_child_node(addr, "prefix-length");
          if (pl && (v = node_value(pl)))
            a.address = std::format("{}/{}", a.address, v);
          ip4.address.push_back(std::move(a));
        }
      }
      if (!ip4.address.empty())
        itf.ipv4 = std::move(ip4);
    }

    // ipv6 augmentation
    n = find_child_node(ch, "ipv6");
    if (n) {
      IetfInterfaces::IetfIpv6 ip6;
      struct lyd_node *mtu6_leaf = find_child_node(n, "mtu");
      if (mtu6_leaf && (v = node_value(mtu6_leaf)))
        ip6.mtu = static_cast<uint32_t>(std::stoul(v));

      for (struct lyd_node *addr = lyd_child(n); addr; addr = addr->next) {
        if (!addr->schema || !addr->schema->name)
          continue;
        if (strcmp(addr->schema->name, "address") != 0)
          continue;
        struct lyd_node *ip_leaf = find_child_node(addr, "ip");
        if (ip_leaf && (v = node_value(ip_leaf))) {
          IetfInterfaces::IetfIpv6::Address a;
          a.address = std::string(v);
          struct lyd_node *pl = find_child_node(addr, "prefix-length");
          if (pl && (v = node_value(pl)))
            a.address = std::format("{}/{}", a.address, v);
          ip6.address.push_back(std::move(a));
        }
      }
      if (!ip6.address.empty())
        itf.ipv6 = std::move(ip6);
    }

    n = find_child_node(ch, "last-change");
    if (n && (v = node_value(n)))
      itf.last_change = std::string(v);

    n = find_child_node(ch, "if-index");
    if (n && (v = node_value(n)))
      itf.if_index = std::stoi(v);

    n = find_child_node(ch, "phys-address");
    if (n && (v = node_value(n)))
      itf.phys_address = std::string(v);

    // leaf-lists: higher-layer-if / lower-layer-if
    for (struct lyd_node *ll = lyd_child(ch); ll; ll = ll->next) {
      if (!ll->schema || !ll->schema->name)
        continue;
      if (strcmp(ll->schema->name, "higher-layer-if") == 0) {
        if ((v = node_value(ll)))
          itf.higher_layer_if.push_back(v);
      }
      if (strcmp(ll->schema->name, "lower-layer-if") == 0) {
        if ((v = node_value(ll)))
          itf.lower_layer_if.push_back(v);
      }
    }

    model->addInterface(std::move(itf));
  }

  return model;
}
