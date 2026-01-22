# Schema-Mount Callback Analysis & Findings

## 🔴 CRITICAL FINDING (2026-01-21 22:59)

### The Paradox

**Our callback CAN find yang-library using lyd_find_path(), but the plugin reports it CANNOT.**

#### Test Results from Callback
```
[extDataCallback] TEST: lyd_find_path(parsed, "/ietf-yang-library:yang-library", ...) returned 0
[extDataCallback] TEST: SUCCESS - found yang-library node at 0x58d17815cd20
[extDataCallback] TEST: Node name = 'yang-library', module = 'ietf-yang-library'
```

#### Plugin Error (Immediately After)
```
libyang[0]: Ext plugin "ly2 schema mount": Could not find 'yang-library' data in 
  "/ietf-network-instance:network-instances/network-instance/vrf-root".
```

### What This Proves

1. ✅ XML structure is CORRECT - lyd_find_path() successfully finds `/ietf-yang-library:yang-library`
2. ✅ Modules exist in parent context (ietf-routing@2018-03-13 = 0x..., ietf-interfaces@2018-02-20 = 0x...)
3. ✅ Tree structure supports path lookup
4. ✅ yang-library node exists and is accessible
5. ❌ Plugin STILL fails when using the SAME data that WE successfully searched

### Hypothesis: Context or Scope Mismatch

**What we do (and it works):**
```c
struct ly_ctx *ctx = ext->module->ctx;  // Parent context
lyd_parse_data_mem(ctx, ext_xml.c_str(), LYD_XML, LYD_PARSE_ONLY, 0, &parsed);
lyd_find_path(parsed, "/ietf-yang-library:yang-library", 0, &test);  // ✅ SUCCESS
*ext_data = parsed;
return LY_SUCCESS;
```

**What plugin does (and it fails):**
```c
// schema_mount.c:383-384 (inline mode) or 484-485 (shared mode)
lyd_find_path(ext_data, "/ietf-yang-library:yang-library", 0, &ext_yl_data);  // ❌ FAILS
```

**Possible reasons:**
1. Context mismatch - plugin uses different context when searching?
2. Wrong return value - should we return `yang-library` node directly instead of tree root `parsed`?
3. Shared vs inline mode - we use shared-schema (line 484-485), not inline (line 383-384)
4. Additional validation after find - plugin may find it but reject it for other reasons
5. Timing issue - something changes between our return and plugin's use?

---

## Complete Status Summary
---

## Complete Status Summary

✅ **Verified Working:**
- Callback executes and returns successfully
- XML parses without errors  
- XML validates with LYD_VALIDATE_PRESENT
- Both required modules exist in parent context
- lyd_find_path() CAN find yang-library in our tree
- Element ordering correct per RFC 8525
- Both new (yang-library) and deprecated (modules-state) formats included

❌ **Still Failing:**
- Plugin reports "Could not find 'yang-library' data" despite our test proving it exists
- Error occurs AFTER callback returns, during plugin's processing of ext_data

🔍 **Next Investigation Priorities:**

1. **HIGH**: Check what node we're returning - should ext_data point to yang-library node or tree root?
2. **HIGH**: Trace shared-schema path (line 484-485) vs inline path (383-384) in schema_mount.c
3. **MEDIUM**: Add more debug to see EXACTLY what the plugin receives
4. **MEDIUM**: Check if there's post-find validation that fails
5. **LOW**: Try returning just yang-library node instead of full tree

---

## Problem Analysis
The schema-mount plugin expects a specific yang-library structure per RFC 8525 (2019-01-04). Our current structure has:
- module-set (correct)
- schema (correct) 
- content-id (correct)
- modules-state (deprecated, may be causing conflict)
- schema-mounts (correct)

## Approaches to Try

### 1. Remove deprecated modules-state
**Priority: HIGH**
- Delete the entire `<modules-state>` element from ext_xml
- modules-state is from ietf-yang-library@2016-06-21 (deprecated)
- yang-library@2019-01-04 uses datastore/schema/module-set structure instead
- This mix may be confusing the plugin

**Status**: TRIED - Causes validation error (mandatory module-set-id missing)
- Even though deprecated, module-set-id is mandatory if modules-state exists in data
- Schema includes deprecated containers for backward compatibility
- Adding back modules-state: plugin still fails even with both formats

**Test**: Remove modules-state entirely and see if plugin accepts the data ✗ FAILED
- modules-state is from ietf-yang-library@2016-06-21 (deprecated)
- yang-library@2019-01-04 uses datastore/schema/module-set structure instead
- This mix may be confusing the plugin

**Test**: Remove modules-state entirely and see if plugin accepts the data

---

### 2. Add datastore element linking schema
**Priority: HIGH**
- RFC 8525 yang-library@2019-01-04 expects datastore-to-schema linkage
- Add `<datastore>` element that references the schema:
```xml
<datastore>
  <name xmlns:ds="urn:ietf:params:xml:ns:yang:ietf-datastores">ds:running</name>
  <schema>vrf-schema</schema>
</datastore>
```
- This tells the plugin which schema applies to which datastore
- For schema-mount, we likely need at least a running datastore reference

**Test**: Add datastore element after schema, before content-id

---

### 3. Fix element ordering per RFC 8525 schema
**Priority: MEDIUM**
- RFC 8525 defines specific ordering in yang-library:
  1. module-set (list)
  2. schema (list)
  3. datastore (list)
  4. content-id (leaf)
- Our current order: content-id, module-set, schema (wrong)
- Reorder to match schema definition

**Test**: Reorder elements to: module-set, schema, [datastore], content-id

---

### 4. Check if import-only-module is needed
**Priority: LOW**
- yang-library@2019-01-04 has import-only-module list
- Some modules might need to be declared as import-only
- ietf-inet-types, ietf-yang-types, etc. might need this

**Test**: Add import-only-module list for dependency modules

---

### 5. Verify module features/deviations
**Priority: LOW**
- yang-library schema allows `<feature>` and `<deviation>` under `<module>`
- We're not declaring any features currently
- If ietf-routing/ietf-interfaces have required features, we might need them

**Test**: Check if mounted modules have features we need to enable

---

### 6. Try inline-schema instead of shared-schema
**Priority: MEDIUM**
- schema-mounts supports two modes:
  - `<shared-schema/>` - uses parent context modules (what we're using)
  - `<inline/>` - separate module set for mounted schema
- Plugin might have issues with shared-schema recognition

**Test**: Change `<shared-schema/>` to `<inline/>` in schema-mounts

---

### 7. Study sysrepo's actual XML output
**Priority: HIGH**
- We copied the structure pattern but may have missed details
- Check what sysrepo actually generates for sr_schema_mount_data_get
- Use sysrepo debugger or intercept actual callback data

**Status**: PENDING - need to capture actual sysrepo output for comparison

**Test**: Compare line-by-line with real sysrepo output

---

### 8. Add yang-library as explicit module in module-set
**Priority: MEDIUM**
- The plugin looks for yang-library "data" but maybe it needs the module listed?
- Add ietf-yang-library itself to the module-set:
```xml
<module>
  <name>ietf-yang-library</name>
  <revision>2019-01-04</revision>
  <namespace>urn:ietf:params:xml:ns:yang:ietf-yang-library</namespace>
</module>
```

**Test**: Include yang-library in module-set

---

### 9. Check namespace declarations
**Priority: LOW**
- Ensure all namespace prefixes are properly declared
- datastore element needs `xmlns:ds` declaration
- Might need explicit xmlns for all modules

**Test**: Add explicit namespace declarations to all elements

---

### 10. Use libyang API to build data tree instead of XML string
**Priority: MEDIUM**
- Instead of building XML string and parsing it
- Use lyd_new_* functions to construct the tree directly
- Eliminates parsing issues, guarantees correct structure

**Test**: Rewrite callback to use lyd_new_path/lyd_new_inner/etc

---

### 11. Check libyang schema-mount plugin source
**Priority: HIGH**
- Look at libyang's schema-mount extension implementation
- File: `src/plugins_exts/schema_mount.c` in libyang repo
- Understand exactly what it's looking for in ext_data

**Status**: REVIEWED - Key findings from schema_mount.c:
- Line 383-384 (inline mode): Uses `lyd_find_path(ext_data, "/ietf-yang-library:yang-library", 0, ...)`
- Line 484 (shared mode): Calls `schema_mount_get_yanglib()` which does same search
- Line 498: Then gets content-id from yang-library data
- **CRITICAL**: For shared-schema, modules listed in yang-library MUST exist in parent context
- Plugin doesn't create new modules, just references existing ones from parent
- Error "Could not find 'yang-library' data" might mean:
  1. Can't find /ietf-yang-library:yang-library path in ext_data, OR
  2. Found yang-library but modules listed don't exist in parent context

**Test**: Read source code to see validation logic ✓ DONE

---

### 12. Try minimal yang-library structure
**Priority: MEDIUM**
- Strip down to absolute minimum required elements
- Just: module-set, schema, content-id, schema-mounts
- No datastore, no modules-state, no extras

**Status**: IMPLEMENTED - Added ly_ctx_get_module() checks in callback
- Callback now verifies both modules exist before building ext_data
- Returns LY_EINVAL if modules not found in parent context
- Debug output shows module pointers

**Test**: Add debug to verify ly_ctx_get_module() finds these modules ✓ DONE

---

### 13. Verify mounted modules are actually available in parent context
**Priority: HIGH**
- shared-schema means modules must exist in parent context
- Check if ietf-routing@2018-03-13 and ietf-interfaces@2018-02-20 are loaded
- Plugin might fail if referencing unavailable modules

**Test**: Add debug to verify ly_ctx_get_module() finds these modules

---

### 14. Add schema-mounts to yang-library namespace
**Priority: LOW**
- Currently schema-mounts is separate root element
- Maybe it should be nested under yang-library?
- Or they should be sibling trees but with explicit relationship?

**Test**: Experiment with structure nesting

---

### 15. Check content-id value
**Priority: LOW**
- We're using "1" as content-id
- Maybe plugin expects content-id to match something specific?
- Or it should be derived from actual context state?

**Test**: Try different content-id values or compute from context

---

## Testing Strategy

### Phase 1: Quick Wins (Do First)
1. ✓ Remove modules-state (tried, caused mandatory child error)
2. ✓ Fix element ordering per RFC 8525 (done: module-set, schema, datastore, content-id)
3. ✓ Add datastore element (done)
4. ✓ Verify modules in parent context (CONFIRMED: both modules exist)

**Conclusion**: Phase 1 complete - modules exist, structure correct, but plugin still fails

### Phase 2: Deep Dive (If Phase 1 Fails)
4. Study libyang schema-mount source code
5. Compare with actual sysrepo output
6. Verify mounted modules availability in parent context

### Phase 3: Structural Changes (If Phase 2 Fails)
7. Rewrite using libyang API instead of XML strings
8. Try inline mode instead of shared-schema
9. Build minimal structure and add incrementally

### Phase 4: Edge Cases (If Phase 3 Fails)
10. Check module features/deviations
11. Add yang-library as module in module-set
12. Experiment with namespace declarations and nesting

## Notes
- Each test should be isolated (one change at a time)
- Keep debug logging to see exact XML structure being rejected
- Document what works/doesn't work for each attempt
- Consider testing with simple lyyang CLI tools outside our app
# Schema-Mount Debugging Findings

## Test Run: 2026-01-21

### What Works
1. ✓ Callback executes successfully
2. ✓ XML parses without errors
3. ✓ XML validates successfully with LYD_VALIDATE_PRESENT
4. ✓ Parent context contains required modules:
   - ietf-routing@2018-03-13 = 0x63d5b3c8aef0
   - ietf-interfaces@2018-02-20 = 0x63d5b3c1a940
5. ✓ Element ordering follows RFC 8525: module-set, schema, datastore, content-id
6. ✓ Both new format (yang-library) and deprecated format (modules-state) included

### What Fails
- Plugin error: `Ext plugin "ly2 schema mount": Could not find 'yang-library' data in "/ietf-network-instance:network-instances/network-instance/vrf-root"`
- This happens AFTER callback returns successfully
- Error occurs when libyang tries to use the ext_data we provided

### Critical Finding
From schema_mount.c line 383-384 (inline mode):
```c
lyd_find_path(ext_data, "/ietf-yang-library:yang-library", 0, (struct lyd_node **)ext_yl_data);
```

The plugin searches for absolute path `/ietf-yang-library:yang-library` in ext_data.

**Our XML structure creates THREE separate root elements:**
```xml
<yang-library xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">...</yang-library>
<modules-state xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-library">...</modules-state>
<schema-mounts xmlns="urn:ietf:params:xml:ns:yang:ietf-yang-schema-mount">...</schema-mounts>
```

**Hypothesis**: When lyd_parse_data_mem() parses this XML with multiple roots, the resulting tree structure might not allow lyd_find_path() to find `/ietf-yang-library:yang-library` correctly.

### Potential Issues

#### Issue 1: Multiple Root Elements
- libyang can parse XML with multiple root elements (they become siblings)
- But lyd_find_path() with absolute path `/ietf-yang-library:yang-library` might only search from the first root
- If yang-library is not the first root, or if siblings aren't searched, path lookup fails

#### Issue 2: Tree Structure
When we parse:
```xml
<yang-library xmlns="...">...</yang-library>
<modules-state xmlns="...">...</modules-state>
<schema-mounts xmlns="...">...</schema-mounts>
```

This creates:
```
root (NULL or first element?)
├── yang-library
├── modules-state (sibling)
└── schema-mounts (sibling)
```

Plugin does: `lyd_find_path(ext_data, "/ietf-yang-library:yang-library", ...)`
- If ext_data points to first element (yang-library), path `/...` might not resolve
- If ext_data points to tree root, but siblings aren't in search path, fails

#### Issue 3: Namespace/Module Context
- All three root elements have different namespaces
- yang-library and modules-state share same namespace (ietf-yang-library)
- schema-mounts has different namespace (ietf-yang-schema-mount)
- Plugin might expect all data under one namespace or module context

## Next Steps to Try

### A. Make yang-library the only root (HIGH PRIORITY)
Nest modules-state and schema-mounts under yang-library or only return yang-library + schema-mounts without modules-state altogether (accept the validation error and parse with different flags).

### B. Use libyang API instead of XML (MEDIUM PRIORITY)
Build the tree with lyd_new_*() functions to ensure correct structure:
```c
struct lyd_node *root = NULL;
lyd_new_path(NULL, ctx, "/ietf-yang-library:yang-library/content-id", "1", 0, &root);
lyd_new_path(root, ctx, "/ietf-yang-library:yang-library/module-set[name='vrf-modules']/name", "vrf-modules", 0, NULL);
// etc...
```

### C. Parse with different flags (LOW PRIORITY)
Try LYD_PARSE_STRICT, LYD_PARSE_NO_STATE, etc. to change tree structure.

### D. Inspect actual tree structure (DEBUG)
After parsing, walk the tree and print what lyd_find_path() can actually find:
```c
struct lyd_node *test;
if (lyd_find_path(parsed, "/ietf-yang-library:yang-library", 0, &test) == LY_SUCCESS) {
    fprintf(stderr, "Found yang-library at %p\n", test);
} else {
    fprintf(stderr, "CANNOT find /ietf-yang-library:yang-library\n");
}
```

## References
- schema_mount.c:383-384 - inline mode yang-library search
- schema_mount.c:484-485 - shared mode yang-library search
- schema_mount.c:498-499 - content-id extraction
- Both modes use same lyd_find_path() approach
