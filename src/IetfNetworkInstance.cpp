#include "IetfNetworkInstance.hpp"
#include "Exceptions.hpp"
#include "IetfInterfaces.hpp"
#include "IetfRouting.hpp"
#include <cstring>
#include <libyang/libyang.h>
#include <cstdlib>
#include <cstdio> // added for logging

using namespace yang;

static struct lyd_node *find_child_by_name_local(struct lyd_node *parent,
                                                 const char *name) {
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
  if (lyd_new_path(nullptr, c, "/ietf-network-instance:network-instances", NULL,
                   0, &root) != LY_SUCCESS) {
    fprintf(stderr, "YangDataError: IetfNetworkInstances::serialize failed to create root node\n");
    return nullptr;
  }

  for (const auto &ni : instances_) {
    std::string pred =
        "network-instances/network-instance[name='" + ni.getName() + "']";
    struct lyd_node *tmp = nullptr;
    if (lyd_new_path(root, c, (pred + "/name").c_str(), ni.getName().c_str(), 0,
                     &tmp) != LY_SUCCESS) {
      fprintf(stderr, "YangDataError: IetfNetworkInstances::serialize failed to create name node for '%s'\n", ni.getName().c_str());
      lyd_free_all(root);
      return nullptr;
    }
    if (!ni.getEnabled()) {
      if (lyd_new_path(root, c, (pred + "/enabled").c_str(), "false", 0,
                       &tmp) != LY_SUCCESS) {
        fprintf(stderr, "YangDataError: IetfNetworkInstances::serialize failed to create enabled node for '%s'\n", ni.getName().c_str());
        lyd_free_all(root);
        return nullptr;
      }
    }
    if (ni.getDescription().has_value()) {
      if (lyd_new_path(root, c, (pred + "/description").c_str(),
                       ni.getDescription()->c_str(), 0, &tmp) != LY_SUCCESS) {
        fprintf(stderr, "YangDataError: IetfNetworkInstances::serialize failed to create description for '%s'\n", ni.getName().c_str());
        lyd_free_all(root);
        return nullptr;
      }
    }
  }

  return root;
}

std::unique_ptr<IetfNetworkInstances>
IetfNetworkInstances::deserialize(const YangContext &ctx,
                                  struct lyd_node *tree) {
  if (!tree) {
    fprintf(stderr, "YangDataError: IetfNetworkInstances::deserialize called with null tree\n");
    return std::make_unique<IetfNetworkInstances>();
  }

  auto model = std::make_unique<IetfNetworkInstances>();

  struct lyd_node *ni_container = nullptr;
  if (tree->schema && tree->schema->name &&
      strcmp(tree->schema->name, "network-instances") == 0 &&
      tree->schema->module && tree->schema->module->name &&
      strcmp(tree->schema->module->name, "ietf-network-instance") == 0) {
    ni_container = tree;
  } else {
    if (lyd_find_path(tree, "/ietf-network-instance:network-instances", 0,
                      &ni_container) != LY_SUCCESS)
      ni_container = nullptr;
  }

  if (!ni_container)
    return model; // empty

  for (struct lyd_node *entry = lyd_child(ni_container); entry;
       entry = entry->next) {
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

      // mount-point containers defined in ietf-network-instance
      // (vrf-root/vsi-root/vv-root)
      if (c->schema->module && c->schema->module->name &&
          strcmp(c->schema->module->name, "ietf-network-instance") == 0) {
        if (strcmp(c->schema->name, "vrf-root") == 0 ||
            strcmp(c->schema->name, "vsi-root") == 0 ||
            strcmp(c->schema->name, "vv-root") == 0) {
          // traverse children of the mount container and dispatch to
          // module-specific parsers
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

LY_ERR IetfNetworkInstances::extDataCallback(const struct lysc_ext_instance *ext,
                                              [[maybe_unused]] const struct lyd_node *node,
                                              [[maybe_unused]] void *user_data,
                                              void **ext_data,
                                              ly_bool *free_ext_data) {
  // This callback provides mount-point metadata (yang-library + schema-mounts) to libyang
  // when it encounters a mount point (vrf-root/vsi-root/vv-root) during validation/parsing.
  // 
  // The instance data contains ONLY the mounted data (e.g., routing config).
  // This callback returns SEPARATE yang-library and schema-mounts trees that describe:
  // - Which modules are available in the mounted schema (yang-library)
  // - How the mount point is configured (schema-mounts with shared-schema)
  
  if (!ext_data || !free_ext_data) {
    fprintf(stderr, "YangError: IetfNetworkInstances::extDataCallback invalid ext_data/free_ext_data pointers\n");
    return LY_EINVAL;
  }

  *ext_data = nullptr;
  *free_ext_data = 0;

  if (!node) {
    fprintf(stderr, "YangError: IetfNetworkInstances::extDataCallback called with null node\n");
    return LY_EINVAL;
  }

  struct ly_ctx *ctx = (struct ly_ctx *)LYD_CTX(node);

  const char *label_name = nullptr;
  if (node->schema && node->schema->name)
    label_name = node->schema->name;
  if (!label_name) {
    fprintf(stderr, "YangError: IetfNetworkInstances::extDataCallback missing label_name\n");
    return LY_EINVAL;
  }

  // Build ext-data XML with BOTH modern yang-library AND deprecated modules-state (with module-set-id)
  // This provides metadata about the mounted schema (ietf-routing, ietf-interfaces) and
  // tells libyang to use shared-schema (parent context) for resolution
  std::string ext_xml = std::string(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    R"(  <yang-library xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">)"
    R"(    <content-id>1</content-id>)"
    R"(    <module-set>)"
    R"(      <name>vrf-modules</name>)"
    R"(      <module>)"
    R"(        <name>ietf-routing</name>)"
    R"(        <revision>2018-03-13</revision>)"
    R"(        <namespace>urn:ietf:params:xml:ns:yang:ietf-routing</namespace>)"
    R"(      </module>)"
    R"(      <module>)"
    R"(        <name>ietf-interfaces</name>)"
    R"(        <revision>2018-02-20</revision>)"
    R"(        <namespace>urn:ietf:params:xml:ns:yang:ietf-interfaces</namespace>)"
    R"(      </module>)"
    R"(    </module-set>)"
    R"(    <schema>)"
    R"(      <name>vrf-schema</name>)"
    R"(      <module-set>vrf-modules</module-set>)"
    R"(    </schema>)"
    R"(  </yang-library>)"
    R"(  <modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">)"
    R"(    <module-set-id>1</module-set-id>)"
    R"(  </modules-state>)"
    R"(  <schema-mounts xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount">)"
    R"(    <mount-point>)"
    R"(      <module>ietf-network-instance</module>)"
    R"(      <label>shitcock</label>)"
    R"(      <shared-schema/>)"
    R"(    </mount-point>)"
    R"(  </schema-mounts>)");

  fprintf(stderr, "ext_data (preparsed XML):\n%s\n", ext_xml.c_str());

  struct lyd_node *parsed = nullptr;
  /* parse as a single XML document (no OPAQ flag), then validate */
  if (lyd_parse_data_mem(ctx, ext_xml.c_str(), LYD_XML, LYD_PARSE_ONLY, 0, &parsed) != LY_SUCCESS) {
    if (parsed) {
      char *printed = nullptr;
      lyd_print_mem(&printed, parsed, LYD_XML, 0);
      fprintf(stderr, "ext_data parse produced partial tree:\n%s\n", printed);
      lyd_free_all(parsed);
    }
    fprintf(stderr, "YangError: IetfNetworkInstances::extDataCallback failed to parse ext_xml\n");
    return LY_EINVAL;
  }
  
  char *printed = nullptr;
  lyd_print_mem(&printed, parsed, LYD_XML, 0);
  fprintf(stderr, "ext_data (after parse XML):\n%s\n", printed);

  if (lyd_validate_all(&parsed, nullptr, LYD_VALIDATE_PRESENT, nullptr) != LY_SUCCESS) {
    char *printed = nullptr;
    lyd_print_mem(&printed, parsed, LYD_XML, 0);
    fprintf(stderr, "ext_data validation failed; parsed ext_data:\n%s\n", printed);
    lyd_free_all(parsed);
    fprintf(stderr, "YangError: IetfNetworkInstances::extDataCallback lyd_validate_all failed\n");
    return LY_EVALID;
  }
  
  lyd_print_mem(&printed, parsed, LYD_XML, 0);
  fprintf(stderr, "ext_data (pretty parsed/validated XML):\n%s\n", printed);
        

  *ext_data = parsed;
  *free_ext_data = 0; // stop changing this to 1 
  return LY_SUCCESS;
}