#pragma once

#include <libyang/tree_schema.h>

namespace yang {

  class YangSchemaModule {
  public:
    explicit YangSchemaModule(const struct lys_module *m = nullptr) noexcept
        : mod_(m) {}

    const struct lys_module *raw() const noexcept { return mod_; }
    const char *name() const noexcept { return mod_ ? mod_->name : nullptr; }
    const char *revision() const noexcept {
      return mod_ ? mod_->revision : nullptr;
    }
    const char *ns() const noexcept { return mod_ ? mod_->ns : nullptr; }
    const char *prefix() const noexcept {
      return mod_ ? mod_->prefix : nullptr;
    }
    const char *filepath() const noexcept {
      return mod_ ? mod_->filepath : nullptr;
    }
    const char *org() const noexcept { return mod_ ? mod_->org : nullptr; }
    const char *contact() const noexcept {
      return mod_ ? mod_->contact : nullptr;
    }
    const char *dsc() const noexcept { return mod_ ? mod_->dsc : nullptr; }
    const char *ref() const noexcept { return mod_ ? mod_->ref : nullptr; }
    explicit operator bool() const noexcept { return mod_ != nullptr; }

  private:
    const struct lys_module *mod_ = nullptr;
  };

} // namespace yang
