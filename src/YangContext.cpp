#include "YangContext.hpp"
#include "Exceptions.hpp"

#include <libyang/libyang.h>

using namespace yang;

YangContext::YangContext(unsigned options) : options_(options) {
  LY_ERR err = ly_ctx_new(NULL, options, &ctx_);
  if (err != LY_SUCCESS) {
    ctx_ = nullptr;
    throw YangError();
  }
}

YangContext::YangContext(struct ly_ctx *raw_ctx) noexcept : ctx_(raw_ctx) {}

YangContext::~YangContext() {
  // XXX this should probably be refcounted.
  // if (ctx_) {
  //   ly_ctx_destroy(ctx_);
  //   ctx_ = nullptr;
  // }
}

void YangContext::addSearchPath(const std::string &path) {
  if (!ctx_)
    return;
  ly_ctx_set_searchdir(ctx_, path.c_str());
  search_paths_.push_back(path);
}

struct lys_module *YangContext::loadModuleInContext(const std::string &name,
                                                    const std::string &revision,
                                                    const char **features) {
  const char *rev = revision.empty() ? nullptr : revision.c_str();
  struct lys_module *mod =
      ly_ctx_load_module(ctx_, name.c_str(), rev, features);
  if (!mod)
    throw YangDataError(*this);
  return mod;
}

const struct lys_module *
YangContext::GetLoadedModuleByName(const std::string &name) const {
  if (!ctx_)
    return nullptr;
  uint32_t idx = 0;
  const struct lys_module *m = nullptr;
  while ((m = ly_ctx_get_module_iter(ctx_, &idx)) != nullptr) {
    if (m->name && name == m->name)
      return m;
  }
  return nullptr;
}

void YangContext::ensureModuleImplemented(const std::string &name,
                                          const std::string &revision) {
  if (!ctx_)
    return;
  const char *rev = revision.empty() ? nullptr : revision.c_str();
  const struct lys_module *mod = ly_ctx_get_module(ctx_, name.c_str(), rev);
  if (mod && !mod->implemented) {
    // libyang 2.x: use lys_set_implemented (accepts optional features array)
    lys_set_implemented(const_cast<struct lys_module *>(mod), nullptr);

    // verify it became implemented; if not, surface an error
    if (!CheckModuleImplemented(name, revision)) {
      throw YangDataError(*this);
    }
  }
}

bool YangContext::CheckModuleImplemented(const std::string &name,
                                         const std::string &revision) const {
  if (!ctx_)
    return false;
  const char *rev = revision.empty() ? nullptr : revision.c_str();
  const struct lys_module *mod = ly_ctx_get_module(ctx_, name.c_str(), rev);
  return (mod && mod->implemented);
}

void YangContext::ensureYangLibraryImplemented() {
  // instance method dispatch to the generic implementer
  ensureModuleImplemented("ietf-yang-library", "");
}

void YangContext::ensureSchemaMountImplemented() {
  // instance method dispatch to the generic implementer
  ensureModuleImplemented("ietf-yang-schema-mount", "");
}
