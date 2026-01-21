#include "IetfInterfaces.hpp"

#include <format>
#include <iostream>
#include <libyang/libyang.h>
#include <regex>
#include <string>

using namespace yang;

#include "IetfInterfaces.hpp"

#include "Exceptions.hpp"
#include <libyang/libyang.h>

using namespace yang;

struct lyd_node *IetfInterfaces::serialize(const YangContext &ctx) const {
  struct ly_ctx *c = ctx.raw();

  struct lyd_node *root = nullptr;
  LY_ERR err =
      lyd_new_path(nullptr, c, "/ietf-interfaces:interfaces", NULL, 0, &root);
  if (err != LY_SUCCESS || !root)
    throw YangDataError(ctx);

  for (const auto &it : ifs_) {
    std::string name = it.name;
    std::string rel_name = std::format("interface[name='{}']/name", name);
    struct lyd_node *tmp = nullptr;

    lyd_new_path(root, c, rel_name.c_str(), name.c_str(), 0, &tmp);

    if (it.description.has_value()) {
      std::string rel_desc =
          std::format("interface[name='{}']/description", name);
      tmp = nullptr;
      lyd_new_path(root, c, rel_desc.c_str(), it.description->c_str(), 0, &tmp);
    }

    if (!it.enabled) {
      std::string rel_en = std::format("interface[name='{}']/enabled", name);
      tmp = nullptr;
      lyd_new_path(root, c, rel_en.c_str(), "false", 0, &tmp);
    }

    if (it.type.has_value()) {
      std::string rel_type = std::format("interface[name='{}']/type", name);
      std::string t =
          std::format("ianaift:{}", yang::ianaIfTypeToString(*it.type));
      tmp = nullptr;
      lyd_new_path(root, c, rel_type.c_str(), t.c_str(), 0, &tmp);
    }

    // Serialize ipv4 addresses
    if (it.ipv4.has_value()) {
      for (const auto &addr : it.ipv4->address) {
        std::string ipstr = addr.address;
        std::string ip_only = ipstr;
        std::string prefix;
        auto slash = ipstr.find('/');
        if (slash != std::string::npos) {
          ip_only = ipstr.substr(0, slash);
          prefix = ipstr.substr(slash + 1);
        }
        std::string base = std::format(
            "interface[name='{}']/ip:ipv4/ip:address[ip='{}']", name, ip_only);
        tmp = nullptr;
        (void)lyd_new_path(root, c, (base + "/ip:ip").c_str(), ip_only.c_str(),
                           0, &tmp);
        if (!prefix.empty()) {
          tmp = nullptr;
          (void)lyd_new_path(root, c, (base + "/ip:prefix-length").c_str(),
                             prefix.c_str(), 0, &tmp);
        }
      }
      // write container-level ipv4 mtu if present (per YANG)
      if (it.ipv4->mtu.has_value()) {
        tmp = nullptr;
        std::string mtu_val = std::format("{}", *it.ipv4->mtu);
        (void)lyd_new_path(
            root, c,
            std::format("interface[name='{}']/ip:ipv4/mtu", name).c_str(),
            mtu_val.c_str(), 0, &tmp);
      }
    }
    // Serialize ipv6 addresses
    if (it.ipv6.has_value()) {
      for (const auto &addr : it.ipv6->address) {
        std::string ipstr = addr.address;
        std::string ip_only = ipstr;
        std::string prefix;
        auto slash = ipstr.find('/');
        if (slash != std::string::npos) {
          ip_only = ipstr.substr(0, slash);
          prefix = ipstr.substr(slash + 1);
        }
        std::string base = std::format(
            "interface[name='{}']/ip:ipv6/ip:address[ip='{}']", name, ip_only);
        tmp = nullptr;
        (void)lyd_new_path(root, c, (base + "/ip:ip").c_str(), ip_only.c_str(),
                           0, &tmp);
        if (!prefix.empty()) {
          tmp = nullptr;
          (void)lyd_new_path(root, c, (base + "/ip:prefix-length").c_str(),
                             prefix.c_str(), 0, &tmp);
        }
      }
      // write container-level ipv6 mtu if present (per YANG)
      if (it.ipv6->mtu.has_value()) {
        tmp = nullptr;
        std::string mtu_val = std::format("{}", *it.ipv6->mtu);
        (void)lyd_new_path(
            root, c,
            std::format("interface[name='{}']/ip:ipv6/mtu", name).c_str(),
            mtu_val.c_str(), 0, &tmp);
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
        LY_SUCCESS)
      ifs = nullptr;
  }
  if (!ifs)
    throw YangDataError(ctx);

  auto find_child = [](struct lyd_node *p, const char *n) -> struct lyd_node * {
    if (!p)
      return nullptr;
    for (struct lyd_node *c = lyd_child(p); c; c = c->next) {
      if (c->schema && c->schema->name && strcmp(c->schema->name, n) == 0)
        return c;
    }
    return nullptr;
  };

  const char *v = nullptr;
  for (struct lyd_node *ch = lyd_child(ifs); ch; ch = ch->next) {
    if (!ch->schema || !ch->schema->name)
      continue;

    if (strcmp(ch->schema->name, "interface") != 0)
      continue;

    IetfInterfaces::IetfInterface itf;
    struct lyd_node *n = find_child(ch, "name");

    if (n && (v = lyd_get_value(n)))
      itf.name = v;

    if (itf.name.empty())
      throw YangDataError(ctx);

    n = find_child(ch, "description");

    if (n && (v = lyd_get_value(n)))
      itf.description = std::string(v);

    n = find_child(ch, "type");

    if (n && (v = lyd_get_value(n))) {
      std::string sv(v);
      // Remove a leading namespace prefix (e.g. "ianaift:") using regex
      static const std::regex rgx(R"(^[^:]+:)");
      sv = std::regex_replace(sv, rgx, "");
      itf.type = yang::ianaIfTypeFromString(sv);
    }

    n = find_child(ch, "enabled");

    if (n && (v = lyd_get_value(n)))
      itf.enabled = !(strcmp(v, "false") == 0 || strcmp(v, "0") == 0);

    n = find_child(ch, "admin-status");

    if (n && (v = lyd_get_value(n)))
      itf.admin_status = std::string(v);

    n = find_child(ch, "oper-status");

    if (n && (v = lyd_get_value(n)))
      itf.oper_status = std::string(v);

    // statistics/discontinuity-time (operational)
    n = find_child(ch, "statistics");
    if (n) {
      struct lyd_node *stat = find_child(n, "discontinuity-time");
      if (stat && (v = lyd_get_value(stat))) {
        IetfInterfaces::IetfInterfaceStatistics s;
        std::string dt(v);
        // XXX Normalize libyang's timezone representation of UTC (+00:00) to
        // 'Z'
        if (dt.size() >= 6 && dt.compare(dt.size() - 6, 6, "+00:00") == 0) {
          dt = dt.substr(0, dt.size() - 6) + "Z";
        }
        s.discontinuity_time = dt;
        itf.statistics = s;
      }
    }

    // ipv4 / ipv6 augmentation (ietf-ip)
    n = find_child(ch, "ipv4");
    if (n) {
      IetfInterfaces::IetfIpv4 ip4;
      /* read container-level mtu per YANG */
      struct lyd_node *mtu_leaf = find_child(n, "mtu");
      if (mtu_leaf && (v = lyd_get_value(mtu_leaf))) {
        ip4.mtu = static_cast<uint32_t>(std::stoul(v));
      }
      for (struct lyd_node *addr = lyd_child(n); addr; addr = addr->next) {
        if (!addr->schema || !addr->schema->name)
          continue;
        if (strcmp(addr->schema->name, "address") != 0)
          continue;
        const char *ipval = nullptr;
        const char *plen = nullptr;
        struct lyd_node *ipleaf = find_child(addr, "ip");
        if (ipleaf && (ipval = lyd_get_value(ipleaf))) {
          IetfInterfaces::IetfIpv4::Address a;
          a.address = std::string(ipval);
          struct lyd_node *pl = find_child(addr, "prefix-length");
          if (pl && (plen = lyd_get_value(pl))) {
            a.address = std::format("{}/{}", ipval, plen);
          }
          ip4.address.push_back(a);
        }
      }
      if (!ip4.address.empty())
        itf.ipv4 = ip4;
    }

    n = find_child(ch, "ipv6");

    if (n) {
      IetfInterfaces::IetfIpv6 ip6;
      /* read container-level mtu per YANG */
      struct lyd_node *mtu6_leaf = find_child(n, "mtu");
      if (mtu6_leaf && (v = lyd_get_value(mtu6_leaf))) {
        ip6.mtu = static_cast<uint32_t>(std::stoul(v));
      }
      for (struct lyd_node *addr = lyd_child(n); addr; addr = addr->next) {
        if (!addr->schema || !addr->schema->name)
          continue;
        if (strcmp(addr->schema->name, "address") != 0)
          continue;
        const char *ipval = nullptr;
        const char *plen = nullptr;
        struct lyd_node *ipleaf = find_child(addr, "ip");
        if (ipleaf && (ipval = lyd_get_value(ipleaf))) {
          IetfInterfaces::IetfIpv6::Address a;
          a.address = std::string(ipval);
          struct lyd_node *pl = find_child(addr, "prefix-length");
          if (pl && (plen = lyd_get_value(pl))) {
            a.address = std::format("{}/{}", ipval, plen);
          }
          ip6.address.push_back(a);
        }
      }
      if (!ip6.address.empty())
        itf.ipv6 = ip6;
    }

    n = find_child(ch, "last-change");

    if (n && (v = lyd_get_value(n)))
      itf.last_change = std::string(v);

    n = find_child(ch, "if-index");

    if (n && (v = lyd_get_value(n)))
      itf.if_index = std::stoi(v);

    n = find_child(ch, "phys-address");

    if (n && (v = lyd_get_value(n)))
      itf.phys_address = std::string(v);

    // higher-layer-if and lower-layer-if are leaf-lists
    for (struct lyd_node *ll = lyd_child(ch); ll; ll = ll->next) {
      if (!ll->schema || !ll->schema->name)
        continue;
      if (strcmp(ll->schema->name, "higher-layer-if") == 0) {
        if ((v = lyd_get_value(ll)))
          itf.higher_layer_if.push_back(v);
      }
      if (strcmp(ll->schema->name, "lower-layer-if") == 0) {
        if ((v = lyd_get_value(ll)))
          itf.lower_layer_if.push_back(v);
      }
    }

    model->addInterface(itf);
  }

  return model;
}
