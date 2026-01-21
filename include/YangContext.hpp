#pragma once

#include <libyang/libyang.h>
#include <string>
#include <vector>

namespace yang {
  // Map libyang context flags (LY_CTX_*) to a scoped enum for callers.
  // Each enum member corresponds to a libyang LY_CTX_* macro (see
  // libyang/context.h). Brief descriptions are included here for convenience.
  enum class YangContextOptions : unsigned {
    None = 0u,

    // LY_CTX_ALL_IMPLEMENTED (0x01)
    // All the imported modules of the schema being parsed are implemented.
    AllImplemented = LY_CTX_ALL_IMPLEMENTED,

    // LY_CTX_REF_IMPLEMENTED (0x02)
    // Implement all imported modules "referenced" from an implemented module
    // (implement modules referenced by when/must/defaults so referenced nodes
    // work).
    RefImplemented = LY_CTX_REF_IMPLEMENTED,

    // LY_CTX_NO_YANGLIBRARY (0x04)
    // Do not internally implement ietf-yang-library module;
    // ly_ctx_get_yanglib_data() will not work until the module is loaded
    // manually.
    NoYanglibrary = LY_CTX_NO_YANGLIBRARY,

    // LY_CTX_DISABLE_SEARCHDIRS (0x08)
    // Do not search for schemas in context searchdirs nor in CWD; caller must
    // provide modules via callbacks (ly_ctx_set_module_imp_clb).
    DisableSearchdirs = LY_CTX_DISABLE_SEARCHDIRS,

    // LY_CTX_DISABLE_SEARCHDIR_CWD (0x10)
    // Do not automatically search the current working directory for schemas.
    DisableSearchdirCwd = LY_CTX_DISABLE_SEARCHDIR_CWD,

    // LY_CTX_PREFER_SEARCHDIRS (0x20)
    // When searching for schema files prefer configured searchdirs over
    // callback.
    PreferSearchdirs = LY_CTX_PREFER_SEARCHDIRS,

    // LY_CTX_EXPLICIT_COMPILE (0x80)
    // Do not automatically compile/update compiled modules on context changes;
    // user must call ly_ctx_compile() to apply changes.
    ExplicitCompile = LY_CTX_EXPLICIT_COMPILE,

    // LY_CTX_SET_PRIV_PARSED (0x40)
    // Use parsed-node private objects for compiled nodes; changing this may
    // require recompilation and imposes constraints on user modifications.
    SetPrivParsed = LY_CTX_SET_PRIV_PARSED,

    // LY_CTX_COMPILE_OBSOLETE (0x0200)
    // Include obsolete nodes in compiled schema so they can be
    // created/validated.
    CompileObsolete = LY_CTX_COMPILE_OBSOLETE,

    // LY_CTX_LYB_HASHES (0x0400)
    // Generate LYB-format hashes for schema nodes (required for LYB I/O).
    LybHashes = LY_CTX_LYB_HASHES,

    // LY_CTX_LEAFREF_EXTENDED (0x0800)
    // Allow extended leafref path expressions (XPath functions like deref()).
    LeafrefExtended = LY_CTX_LEAFREF_EXTENDED,

    // LY_CTX_LEAFREF_LINKING (0x1000)
    // Link valid leafref nodes with their targets during validation
    // and enable related helper APIs.
    LeafrefLinking = LY_CTX_LEAFREF_LINKING,

    // LY_CTX_BUILTIN_PLUGINS_ONLY (0x2000)
    // Use only built-in YANG type plugins (treat derived types as base types).
    BuiltinPluginsOnly = LY_CTX_BUILTIN_PLUGINS_ONLY,

    // LY_CTX_STATIC_PLUGINS_ONLY (0x4000)
    // Restrict loading to static (built-in) plugins and ignore external plugin
    // dirs.
    StaticPluginsOnly = LY_CTX_STATIC_PLUGINS_ONLY,

    // LY_CTX_ENABLE_IMP_FEATURES (0x0100)
    // Enable all features of newly implemented imported modules by default.
    EnableImpFeatures = LY_CTX_ENABLE_IMP_FEATURES
  };

  using YangContextOption = YangContextOptions;

  inline constexpr unsigned toFlags(YangContextOptions o) noexcept {
    return static_cast<unsigned>(o);
  }

  class YangContext {
  public:
    YangContext(unsigned options = 0);
    explicit YangContext(struct ly_ctx *raw_ctx) noexcept;
    ~YangContext();

    YangContext(const YangContext &) = delete;
    YangContext &operator=(const YangContext &) = delete;

    void addSearchPath(const std::string &path);
    struct lys_module *loadModuleInContext(const std::string &name,
                                           const std::string &revision,
                                           const char **features = nullptr);
    const std::vector<std::string> &searchPaths() const noexcept {
      return search_paths_;
    }

    struct ly_ctx *raw() const noexcept { return ctx_; }

    // Find a loaded module by name; returns nullptr if not found.
    const struct lys_module *
    GetLoadedModuleByName(const std::string &name) const;

    // typedef for our callback type
    using ExtDataCallback = void *(*)(const struct lys_ext_instance *, void *);

    // Register an ext-data callback with the underlying libyang context.
    // The provided user_data is passed through to the callback by libyang.
    void registerExtDataCallback(ly_ext_data_clb cb, void *user_data) {
      if (raw()) {
        ly_ctx_set_ext_data_clb(raw(), cb, user_data);
      }
    }

  private:
    struct ly_ctx *ctx_ = nullptr;
    std::vector<std::string> search_paths_;
    unsigned options_ = 0;
  };

} // namespace yang
