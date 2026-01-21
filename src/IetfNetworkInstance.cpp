#include "IetfNetworkInstance.hpp"
#include "Exceptions.hpp"
#include <libyang/libyang.h>
#include <cstring>
#include "IetfRouting.hpp"
#include "IetfInterfaces.hpp"

using namespace yang;

static struct lyd_node *find_child_by_name_local(struct lyd_node *parent, const char *name) {
  if (!parent)
    return nullptr;
  for (struct lyd_node *c = lyd_child(parent); c; c = c->next) {
    if (c->schema && c->schema->name && strcmp(c->schema->name, name) == 0)
      return c;
  }
  return nullptr;
}

static const char *get_node_value_local(struct lyd_node *n) {
  if (!n)
    return nullptr;
  return lyd_get_value(n);
}

struct lyd_node *IetfNetworkInstances::serialize(const YangContext &ctx) const {
  struct ly_ctx *c = ctx.raw();
  struct lyd_node *root = nullptr;
  if (lyd_new_path(nullptr, c, "/ietf-network-instance:network-instances", NULL, 0, &root) != LY_SUCCESS)
    throw YangDataError(ctx);

  for (const auto &ni : instances_) {
    std::string pred = "network-instances/network-instance[name='" + ni.getName() + "']";
    struct lyd_node *tmp = nullptr;
    if (lyd_new_path(root, c, (pred + "/name").c_str(), ni.getName().c_str(), 0, &tmp) != LY_SUCCESS)
      throw YangDataError(ctx);
    if (!ni.getEnabled()) {
      if (lyd_new_path(root, c, (pred + "/enabled").c_str(), "false", 0, &tmp) != LY_SUCCESS)
        throw YangDataError(ctx);
    }
    if (ni.getDescription().has_value()) {
      if (lyd_new_path(root, c, (pred + "/description").c_str(), ni.getDescription()->c_str(), 0, &tmp) != LY_SUCCESS)
        throw YangDataError(ctx);
    }
  }

  return root;
}

std::unique_ptr<IetfNetworkInstances> IetfNetworkInstances::deserialize(const YangContext &ctx, struct lyd_node *tree) {
  if (!tree)
    throw YangDataError(ctx);

  auto model = std::make_unique<IetfNetworkInstances>();

  struct lyd_node *ni_container = nullptr;
  if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "network-instances") == 0 &&
      tree->schema->module && tree->schema->module->name &&
      strcmp(tree->schema->module->name, "ietf-network-instance") == 0) {
    ni_container = tree;
  } else {
    if (lyd_find_path(tree, "/ietf-network-instance:network-instances", 0, &ni_container) != LY_SUCCESS)
      ni_container = nullptr;
  }

  if (!ni_container)
    return model; // empty

  for (struct lyd_node *entry = lyd_child(ni_container); entry; entry = entry->next) {
    if (!entry->schema || !entry->schema->name)
      continue;
    if (strcmp(entry->schema->name, "network-instance") != 0)
      continue;

    IetfNetworkInstances::NetworkInstance ni;
    const char *v = nullptr;

    // iterate direct children of the network-instance
    for (struct lyd_node *c = lyd_child(entry); c; c = c->next) {
      if (!c->schema || !c->schema->name)
        continue;

      // simple leaves
      if (strcmp(c->schema->name, "name") == 0) {
        if ((v = get_node_value_local(c)))
          ni.setName(v);
        continue;
      }
      if (strcmp(c->schema->name, "enabled") == 0) {
        if ((v = get_node_value_local(c)))
          ni.setEnabled((strcmp(v, "true") == 0 || strcmp(v, "1") == 0));
        else
          ni.setEnabled(true);
        continue;
      }
      if (strcmp(c->schema->name, "description") == 0) {
        if ((v = get_node_value_local(c)))
          ni.setDescription(std::string(v));
        continue;
      }

      // mount-point containers defined in ietf-network-instance (vrf-root/vsi-root/vv-root)
      if (c->schema->module && c->schema->module->name &&
          strcmp(c->schema->module->name, "ietf-network-instance") == 0) {
        if (strcmp(c->schema->name, "vrf-root") == 0 ||
            strcmp(c->schema->name, "vsi-root") == 0 ||
            strcmp(c->schema->name, "vv-root") == 0) {
          // traverse children of the mount container and dispatch to module-specific parsers
          for (struct lyd_node *mc = lyd_child(c); mc; mc = mc->next) {
            if (!mc->schema || !mc->schema->module || !mc->schema->module->name)
              continue;
            const char *mod = mc->schema->module->name;
            if (strcmp(mod, "ietf-routing") == 0) {
              // call deserialize and store result
              auto routing = IetfRouting::deserialize(ctx, mc);
              if (routing)
                ni.setRouting(std::move(routing));
            } else if (strcmp(mod, "ietf-interfaces") == 0) {
              auto ifs = IetfInterfaces::deserialize(ctx, mc);
              if (ifs)
                ni.setInterfaces(std::move(ifs));
            }
          }
        }
      }
    } // end children loop

    if (!ni.getName().empty())
      model->mutableNetworkInstances().push_back(std::move(ni));
  }

  return model;
}
