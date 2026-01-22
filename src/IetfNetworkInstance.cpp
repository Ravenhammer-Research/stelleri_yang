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
  // Recursion guard (matches common.c pattern)
  static thread_local int mount_cb_r = 0;
  
  fprintf(stderr, "[extDataCallback] Called (node=%p, mount_cb_r=%d)\n", (void*)node, mount_cb_r);
  
  *ext_data = nullptr;
  *free_ext_data = 0;

  // Check for recursive call
  if (mount_cb_r) {
    fprintf(stderr, "[extDataCallback] Recursive call detected, returning LY_SUCCESS\n");
    return LY_SUCCESS;
  }

  // Validate this is a mount-point extension (matches common.c check)
  if (!ext || !ext->def || !ext->def->module || !ext->def->module->name ||
      strcmp(ext->def->module->name, "ietf-yang-schema-mount") != 0 ||
      !ext->def->name || strcmp(ext->def->name, "mount-point") != 0) {
    fprintf(stderr, "[extDataCallback] Not a mount-point extension, returning LY_EINVAL\n");
    return LY_EINVAL;
  }

  fprintf(stderr, "[extDataCallback] Valid mount-point extension (module=%s, name=%s)\n", 
          ext->def->module->name, ext->def->name);

  // Compute parent path (matches common.c logic)
  char *parent_path = nullptr;
  if (node) {
    // Parsing data, use the data node
    parent_path = lyd_path(node, LYD_PATH_STD, NULL, 0);
    fprintf(stderr, "[extDataCallback] Using data node path\n");
  } else {
    // Creating shared contexts, use the ext schema parent node
    if (ext->parent_stmt != LY_STMT_CONTAINER && ext->parent_stmt != LY_STMT_LIST) {
      fprintf(stderr, "[extDataCallback] Invalid parent_stmt type, returning LY_EINVAL\n");
      return LY_EINVAL;
    }
    parent_path = lysc_path((const struct lysc_node *)ext->parent, LYSC_PATH_DATA, NULL, 0);
    fprintf(stderr, "[extDataCallback] Using schema parent path\n");
  }
  if (!parent_path) {
    fprintf(stderr, "[extDataCallback] Failed to get parent_path, returning LY_EMEM\n");
    return LY_EMEM;
  }

  fprintf(stderr, "[extDataCallback] parent_path = %s\n", parent_path);

  // Extract label from parent_path (last component is the mount label)
  const char *label_name = strrchr(parent_path, '/');
  if (label_name) {
    label_name++; // skip the '/'
  } else {
    label_name = parent_path;
  }
  
  // Remove any predicates from label
  std::string label(label_name);
  size_t pred_pos = label.find('[');
  if (pred_pos != std::string::npos) {
    label = label.substr(0, pred_pos);
  }
  
  // Remove namespace prefix if present (label must be local name only)
  size_t colon_pos = label.find(':');
  if (colon_pos != std::string::npos) {
    label = label.substr(colon_pos + 1);
  }

  fprintf(stderr, "[extDataCallback] Extracted label = '%s'\n", label.c_str());

  struct ly_ctx *ctx = ext->module->ctx;
  
  // For shared-schema mode, verify that the modules we're declaring actually exist in parent context
  // This is CRITICAL - shared-schema doesn't create new modules, it references existing ones
  const struct lys_module *routing_mod = ly_ctx_get_module(ctx, "ietf-routing", "2018-03-13");
  const struct lys_module *interfaces_mod = ly_ctx_get_module(ctx, "ietf-interfaces", "2018-02-20");
  fprintf(stderr, "[extDataCallback] Parent context module check: ietf-routing@2018-03-13 = %p, ietf-interfaces@2018-02-20 = %p\n",
          (void*)routing_mod, (void*)interfaces_mod);
  if (!routing_mod || !interfaces_mod) {
    fprintf(stderr, "[extDataCallback] ERROR: shared-schema requires modules to exist in parent context!\n");
    free(parent_path);
    return LY_EINVAL;
  }
  
  // Build ext-data XML with correct label (matches sysrepo structure)
  // NOTE: Must include BOTH new (datastore/schema/module-set) AND deprecated (modules-state)
  // formats. Even though modules-state is deprecated in RFC 8525 (2019-01-04), it's still
  // in the schema with mandatory module-set-id child. Validation fails without it.
  // The schema-mount plugin should use the new structure and ignore the deprecated part.
  // Added datastore element per RFC 8525 - links running datastore to schema.
  // Element ordering follows RFC 8525: module-set, schema, datastore, content-id.
  std::string ext_xml = std::string(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    R"(<yang-library xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library" xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">)"
    R"(<module-set>)"
    R"(<name>vrf-modules</name>)"
    R"(<module>)"
    R"(<name>ietf-routing</name>)"
    R"(<revision>2018-03-13</revision>)"
    R"(<namespace>urn:ietf:params:xml:ns:yang:ietf-routing</namespace>)"
    R"(</module>)"
    R"(<module>)"
    R"(<name>ietf-interfaces</name>)"
    R"(<revision>2018-02-20</revision>)"
    R"(<namespace>urn:ietf:params:xml:ns:yang:ietf-interfaces</namespace>)"
    R"(</module>)"
    R"(</module-set>)"
    R"(<schema>)"
    R"(<name>vrf-schema</name>)"
    R"(<module-set>vrf-modules</module-set>)"
    R"(</schema>)"
    R"(<datastore>)"
    R"(<name>ds:running</name>)"
    R"(<schema>vrf-schema</schema>)"
    R"(</datastore>)"
    R"(<content-id>1</content-id>)"
    R"(</yang-library>)"
    R"(<modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">)"
    R"(<module-set-id>1</module-set-id>)"
    R"(</modules-state>)"
    R"(<schema-mounts xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount">)"
    R"(<mount-point>)"
    R"(<module>ietf-network-instance</module>)"
    R"(<label>)" + label + R"(</label>)"
    R"(<inline/>)"
    R"(</mount-point>)"
    R"(</schema-mounts>)");

  struct lyd_node *parsed = nullptr;
  
  fprintf(stderr, "[extDataCallback] Parsing ext-data XML (length=%zu)\n", ext_xml.length());
  
  // Set recursion guard before parsing
  mount_cb_r = 1;
  LY_ERR rc = lyd_parse_data_mem(ctx, ext_xml.c_str(), LYD_XML, LYD_PARSE_ONLY, 0, &parsed);
  mount_cb_r = 0;
  
  if (rc != LY_SUCCESS) {
    fprintf(stderr, "[extDataCallback] Parse failed with rc=%d\n", rc);
    free(parent_path);
    if (parsed) {
      lyd_free_all(parsed);
    }
    return LY_EINVAL;
  }

  fprintf(stderr, "[extDataCallback] Parse successful, validating...\n");

  // Validate the parsed data
  mount_cb_r = 1;
  rc = lyd_validate_all(&parsed, nullptr, LYD_VALIDATE_PRESENT, nullptr);
  mount_cb_r = 0;
  
  if (rc != LY_SUCCESS) {
    fprintf(stderr, "[extDataCallback] Validation failed with rc=%d\n", rc);
    free(parent_path);
    lyd_free_all(parsed);
    return LY_EVALID;
  }

  fprintf(stderr, "[extDataCallback] Validation successful, returning ext_data\n");
  
  // CRITICAL DEBUG: Test if lyd_find_path can find yang-library in our parsed tree
  // This is EXACTLY what the schema-mount plugin does (schema_mount.c:383-384)
  struct lyd_node *test_find = nullptr;
  LY_ERR find_rc = lyd_find_path(parsed, "/ietf-yang-library:yang-library", 0, &test_find);
  fprintf(stderr, "[extDataCallback] TEST: lyd_find_path(parsed, \"/ietf-yang-library:yang-library\", ...) returned %d\n", find_rc);
  if (find_rc == LY_SUCCESS) {
    fprintf(stderr, "[extDataCallback] TEST: SUCCESS - found yang-library node at %p\n", (void*)test_find);
    if (test_find && test_find->schema) {
      fprintf(stderr, "[extDataCallback] TEST: Node name = '%s', module = '%s'\n", 
              test_find->schema->name, test_find->schema->module->name);
    }
  } else {
    fprintf(stderr, "[extDataCallback] TEST: FAILED - lyd_find_path could not find yang-library (rc=%d)\n", find_rc);
    fprintf(stderr, "[extDataCallback] TEST: This is why the plugin fails! The path lookup doesn't work on our tree structure.\n");
  }
  
  // Print the parsed data tree before returning
  fprintf(stderr, "[extDataCallback] Parsed ext_data tree:\n");
  lyd_print_file(stderr, parsed, LYD_XML, LYD_PRINT_SIBLINGS);
  fprintf(stderr, "\n[extDataCallback] End of parsed tree\n");

  // Return the tree root (parsed), matching common.c behavior
  // The plugin will search for yang-library within this tree
  fprintf(stderr, "[extDataCallback] Returning tree root: parsed=%p, name='%s', module='%s'\n",
          (void*)parsed,
          parsed->schema ? parsed->schema->name : "NULL", 
          parsed->schema && parsed->schema->module ? parsed->schema->module->name : "NULL");
  
  *ext_data = parsed;
  *free_ext_data = 1;  // Match common.c: libyang will free the data
  
  free(parent_path);
  return LY_SUCCESS;
}