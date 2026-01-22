#include "IetfNetworkInstance.hpp"
#include "Exceptions.hpp"
#include "IetfInterfaces.hpp"
#include "IetfRouting.hpp"
#include <cstring>
#include <libyang/libyang.h>
#include <cstdlib>
#include <cstdio> // added for logging

using namespace yang;

// Debug helper function for ext-data callback
static void debug_print_ext_data_tree(struct lyd_node *data, const char *label) {
  fprintf(stderr, "[extDataCallback] %s:\n", label);
  for (struct lyd_node *child = data; child; child = child->next) {
    if (child->schema && child->schema->name) {
      fprintf(stderr, "  - Root: %s (module: %s)\n", 
              child->schema->name,
              child->schema->module ? child->schema->module->name : "NULL");
      for (struct lyd_node *sub = lyd_child(child); sub; sub = sub->next) {
        if (sub->schema && sub->schema->name) {
          fprintf(stderr, "    - Child: %s = %s\n", 
                  sub->schema->name,
                  lyd_get_value(sub) ? lyd_get_value(sub) : "(container)");
        }
      }
    }
  }
}

// Validate ext-data with debug logging
static LY_ERR validate_ext_data(struct lyd_node **data, struct ly_ctx *ctx) {
  fprintf(stderr, "[extDataCallback] Validating ext-data...\n");
  LY_ERR rc = lyd_validate_all(data, ctx, LYD_VALIDATE_PRESENT, nullptr);
  fprintf(stderr, "[extDataCallback] Validation result: rc=%d\n", rc);

  if (rc != LY_SUCCESS) {
    fprintf(stderr, "[extDataCallback] ERROR: Failed to validate ext-data\n");
    
    // Print detailed error info
    const struct ly_err_item *err = ly_err_first(ctx);
    while (err) {
      fprintf(stderr, "[extDataCallback]   Error: %s (path: %s)\n", 
              err->msg ? err->msg : "NULL",
              err->data_path ? err->data_path : "NULL");
      err = err->next;
    }
    
    lyd_free_all(*data);
    *data = nullptr;
    return rc;
  }

  fprintf(stderr, "[extDataCallback] Validation successful\n");
  debug_print_ext_data_tree(*data, "Validated data tree structure");
  return LY_SUCCESS;
}

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

/*
 *  ly_ext_data_clb, set by ly_ctx_set_ext_data_clb
 * Callback for getting arbitrary run-time data required by an extension instance.
 * Parameters
 *   [in]	ext	Compiled extension instance.
 *   [in]	parent	Data parent node instance of a schema node with ext instance. In special cases (when not working with data) it can be NULL!
 *   [in]	user_data	User-supplied callback data.
 *   [out]	ext_data	Provided extension instance data.
    [out]	ext_data_free	Whether the extension instance should free ext_data or not.
 * Returns
 *   LY_ERR value. 
 * Definition at line 536 of file context.h.
 *
 * struct lysc_ext_instance
 * YANG extension compiled instance.
 * Definition at line 436 of file plugins_exts.h.
 * Data Fields
 * const char * 	argument 	
 * optional value of the extension's argument
 * void * 	compiled 	    
 * private plugin compiled data
 * struct lysc_ext * 	def 	
 * pointer to the extension definition
 * struct lysc_ext_instance * 	exts 	
 * list of the extension instances (sized array)
 * struct lys_module * 	module 	
 * module where the extension instantiated is defined
 * void * 	parent 	
 * pointer to the parent element holding the extension instance(s), use lysc_ext_instance::parent_stmt to access the value/structure
 * enum ly_stmt 	parent_stmt 	
 * type of the parent statement
 * uint64_t 	parent_stmt_index 	
 * index of the stamenet in case the parent does not point to the parent statement directly and it is an array
 * struct lysc_ext_substmt * 	substmts 	
 * list of supported known YANG statements with the pointer to their compiled data (sized array) 
 */
LY_ERR IetfNetworkInstances::extDataCallback(const struct lysc_ext_instance *ext,
                                              const struct lyd_node *node,
                                              [[maybe_unused]] void *user_data,
                                              void **ext_data,
                                              ly_bool *free_ext_data) {
  // Prevent recursion during ext-data parsing
  static thread_local int recursion_guard = 0;
  
  fprintf(stderr, "\n=== [extDataCallback] ENTRY ===\n");
  fprintf(stderr, "[extDataCallback] recursion_guard=%d\n", recursion_guard);
  
  *ext_data = nullptr;
  *free_ext_data = 0;

  if (recursion_guard) {
    fprintf(stderr, "[extDataCallback] Recursion detected, returning SUCCESS\n");
    return LY_SUCCESS;
  }

  // Validate this is a mount-point extension from ietf-yang-schema-mount
  if (!ext || !ext->def || !ext->def->module || !ext->def->name) {
    fprintf(stderr, "[extDataCallback] ERROR: Invalid ext structure\n");
    return LY_EINVAL;
  }
  
  fprintf(stderr, "[extDataCallback] ext->def->module->name=%s\n", ext->def->module->name);
  fprintf(stderr, "[extDataCallback] ext->def->name=%s\n", ext->def->name);
  
  if (strcmp(ext->def->module->name, "ietf-yang-schema-mount") != 0 ||
      strcmp(ext->def->name, "mount-point") != 0) {
    fprintf(stderr, "[extDataCallback] Not a mount-point extension, ignoring\n");
    return LY_EINVAL;
  }

  // The extension parent should be a container (vrf-root, vsi-root, or vv-root)
  fprintf(stderr, "[extDataCallback] ext->parent_stmt=%d (CONTAINER=%d, LIST=%d)\n", 
          ext->parent_stmt, LY_STMT_CONTAINER, LY_STMT_LIST);
  
  if (ext->parent_stmt != LY_STMT_CONTAINER && ext->parent_stmt != LY_STMT_LIST) {
    fprintf(stderr, "[extDataCallback] ERROR: Invalid parent_stmt\n");
    return LY_EINVAL;
  }

  // Get the mount point label from the extension's argument
  const char *label = ext->argument;
  fprintf(stderr, "[extDataCallback] ext->argument=%s\n", label ? label : "NULL");
  
  if (!label || strlen(label) == 0) {
    // Fall back to extracting from schema node name
    const struct lysc_node *parent_node = (const struct lysc_node *)ext->parent;
    label = parent_node->name;
    fprintf(stderr, "[extDataCallback] Using parent node name as label: %s\n", label ? label : "NULL");
  }

  if (!label) {
    fprintf(stderr, "[extDataCallback] ERROR: No label found\n");
    return LY_EINVAL;
  }

  fprintf(stderr, "[extDataCallback] Final mount-point label: %s\n", label);
  
  if (node) {
    char *node_path = lyd_path(node, LYD_PATH_STD, NULL, 0);
    fprintf(stderr, "[extDataCallback] Data node path: %s\n", node_path ? node_path : "NULL");
    free(node_path);
  } else {
    fprintf(stderr, "[extDataCallback] No data node provided\n");
  }

  struct ly_ctx *ctx = ext->module->ctx;
  fprintf(stderr, "[extDataCallback] Context: %p\n", (void*)ctx);

  // Build the ext-data response per RFC 8528
  // This must include:
  // 1. yang-library (RFC 8525) describing the mounted schema
  // 2. schema-mounts describing mount point configuration
  
  fprintf(stderr, "[extDataCallback] Building ext-data XML...\n");
  
  std::string xml =
    "<yang-library xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
      "<module-set>"
        "<name>vrf-modules</name>"
        "<module>"
          "<name>ietf-routing</name>"
          "<revision>2018-03-13</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-routing</namespace>"
        "</module>"
        "<module>"
          "<name>ietf-ipv4-unicast-routing</name>"
          "<revision>2018-03-13</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-ipv4-unicast-routing</namespace>"
        "</module>"
        "<module>"
          "<name>ietf-ipv6-unicast-routing</name>"
          "<revision>2018-03-13</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-ipv6-unicast-routing</namespace>"
        "</module>"
        "<module>"
          "<name>ietf-interfaces</name>"
          "<revision>2018-02-20</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-interfaces</namespace>"
        "</module>"
        "<module>"
          "<name>ietf-ip</name>"
          "<revision>2018-02-22</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-ip</namespace>"
        "</module>"
        "<import-only-module>"
          "<name>ietf-inet-types</name>"
          "<revision>2013-07-15</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-inet-types</namespace>"
        "</import-only-module>"
        "<import-only-module>"
          "<name>ietf-yang-types</name>"
          "<revision>2013-07-15</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:ietf-yang-types</namespace>"
        "</import-only-module>"
        "<import-only-module>"
          "<name>iana-if-type</name>"
          "<revision>2023-01-26</revision>"
          "<namespace>urn:ietf:params:xml:ns:yang:iana-if-type</namespace>"
        "</import-only-module>"
      "</module-set>"
      "<schema>"
        "<name>vrf-schema</name>"
        "<module-set>vrf-modules</module-set>"
      "</schema>"
      "<datastore xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">"
        "<name>ds:running</name>"
        "<schema>vrf-schema</schema>"
      "</datastore>"
      "<datastore xmlns:ds=\"urn:ietf:params:xml:ns:yang:ietf-datastores\">"
        "<name>ds:operational</name>"
        "<schema>vrf-schema</schema>"
      "</datastore>"
      "<content-id>1</content-id>"
    "</yang-library>"
    "<modules-state xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-library\">"
      "<module-set-id>1</module-set-id>"
    "</modules-state>"
    "<schema-mounts xmlns=\"urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount\">"
      "<mount-point>"
        "<module>ietf-network-instance</module>"
        "<label>" + std::string(label) + "</label>"
        "<inline/>"
      "</mount-point>"
    "</schema-mounts>";

  fprintf(stderr, "[extDataCallback] XML length: %zu bytes\n", xml.length());
  fprintf(stderr, "[extDataCallback] XML content:\n%s\n", xml.c_str());

  // Parse the ext-data with recursion guard
  fprintf(stderr, "[extDataCallback] Setting recursion_guard=1 and parsing...\n");
  recursion_guard = 1;
  struct lyd_node *data = nullptr;
  LY_ERR rc = lyd_parse_data_mem(ctx, xml.c_str(), LYD_XML,
                                  LYD_PARSE_STRICT | LYD_PARSE_ONLY, 0, &data);
  recursion_guard = 0;
  fprintf(stderr, "[extDataCallback] Parse complete, recursion_guard=0, rc=%d\n", rc);

  if (rc != LY_SUCCESS) {
    fprintf(stderr, "[extDataCallback] ERROR: Failed to parse ext-data XML: %d\n", rc);
    if (data) {
      fprintf(stderr, "[extDataCallback] Cleaning up partial data tree\n");
      lyd_free_all(data);
    }
    return rc;
  }

  if (!data) {
    fprintf(stderr, "[extDataCallback] ERROR: No data returned from parsing\n");
    return LY_EINVAL;
  }

  fprintf(stderr, "[extDataCallback] Parse successful, data=%p\n", (void*)data);
  debug_print_ext_data_tree(data, "Parsed data tree");

  // Validate the ext-data
  rc = validate_ext_data(&data, ctx);
  if (rc != LY_SUCCESS) {
    return rc;
  }

  // Return the parsed ext-data to libyang
  *ext_data = data;
  *free_ext_data = 1;
  fprintf(stderr, "[extDataCallback] Returning SUCCESS with ext_data=%p\n", (void*)data);
  fprintf(stderr, "=== [extDataCallback] EXIT SUCCESS ===\n\n");
  return LY_SUCCESS;
}