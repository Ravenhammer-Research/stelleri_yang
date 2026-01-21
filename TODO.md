TODO: identityref handling and IetfRouting improvements

Background
- Problem: `rib.address_family` in `IetfRouting::deserialize` is read from an `identityref` value returned by `lyd_get_value()` which may include an XML QName prefix (e.g. "ietf-ip:ipv4"). Current code strips the prefix with a simple `find(':')` + `substr` to normalize to the local-name ("ipv4").

Why the prefix appears
- Per the YANG <-> XML mapping, `identityref` values are encoded as QNames in XML. QNames may be "localname" or "prefix:localname" where the prefix maps to a namespace URI via `xmlns`.
- Libyang exposes the lexical value as it appears in the XML; that lexical form can include the prefix used in the document.

Why the existing strip was added (pragmatic fix)
- Stripping the prefix normalizes both prefixed and unprefixed forms to the bare identity local-name. This matches existing tests and other parts of the code that compare the identity by its local-name.

Drawbacks of the string-strip approach
- Loss of module/namespace context: different modules could define identities with the same local-name; dropping the prefix loses which module the identity came from.
- The XML prefix is an alias; naive stripping ignores whether the prefix actually maps to the intended module namespace (though it usually does in well-formed XML).
- Future features that need exact identity resolution or cross-module comparisons will be fragile.
- its ghetto

Better approaches (recommended)
1) Resolve QName using libyang and the parsed tree's namespace mapping
   - Parse the lexical QName into (prefix, local-name).
   - Use the prefix->namespace mapping from the parsed data node (libyang exposes namespace info on nodes) to obtain the namespace URI.
   - From the libyang context (`ly_ctx`), map the namespace URI to the module, and then validate/resolve the identity in that module.
   - Store both `module` (or namespace URI) and `local-name`, or store a canonical form like `module:identity`.

2) Store the resolved libyang identity (most robust)
   - After resolution, find the `struct lys_module` and the `lys_ident`/identity definition and either keep a reference/identifier to that object or record `module->name` + identity name.
   - This preserves full semantics and prevents ambiguity.

3) Keep the simple strip but add guardrails (pragmatic short-term)
   - Continue storing only the local-name but add a code comment explaining the limitation and add tests asserting no conflicting identity names are expected.
   - Add a ticket/note to move to proper resolution if cross-module ambiguity is possible.

Recommended next steps
- Short-term (fast): keep current behavior for tests, but add a clear TODO and comments (this file documents rationale).
- Medium-term (recommended): implement approach (1) — a helper that resolves QNames via libyang APIs:
  - Helper API sketch: `std::pair<std::string /*module*/, std::string /*local*/> resolve_qname(struct lyd_node *ctx_node, const char *qname, struct ly_ctx *ctx)`
  - Use `lyxml_ctx`/node namespace mapping or `lyd_node`/`lysc` fields to map prefix -> namespace, then `ly_ctx_get_module_implemented()`/`ly_ctx_get_module()` equivalents to find the module by namespace.
  - Validate the identity exists in the module, and return `(module_name, local_name)`.
- Long-term: store a typed representation (module+identity) in `IetfRouting::Rib::address_family` or a small struct instead of plain `std::string`.

Testing and validation
- Add unit tests that exercise both prefixed and unprefixed identityref values (e.g., `ipv4` and `ietf-ip:ipv4`) when parsing.
- Validate serialization preserves chosen canonical form (decide whether to serialize with or without prefix; use libyang to construct nodes with `lyd_new_path` given module+identity).

Files to change (candidates)
- `src/IetfRouting.cpp` — replace naive strip with `resolve_qname()` usage.
- `include/IetfRouting.hpp` — consider changing `Rib::address_family` to a small struct `{ std::string module; std::string name; }` or a dedicated type.
- `tests/TestIetfRouting.cpp` — add tests for prefixed/unprefixed identityref lexical forms and roundtrip behavior.

Action priority
1. Add this TODO (done).
2. Implement `resolve_qname()` helper and update `IetfRouting::deserialize`.
3. Add unit tests for both input forms.
4. Optionally change the stored type for address-family to contain module+name.

Notes
- Using libyang to resolve QNames is the robust approach; avoid brittle string manipulation when module disambiguation matters.
- The existing pragmatic strip is acceptable short-term if you can guarantee no colliding identity local-names across imported modules in your use-case.
