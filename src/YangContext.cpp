#include "YangContext.hpp"
#include "YangSchemaModule.hpp"
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

YangContext::YangContext(struct ly_ctx* raw_ctx) noexcept : ctx_(raw_ctx) {}

YangContext::~YangContext() {
    if (ctx_) {
        ly_ctx_destroy(ctx_);
        ctx_ = nullptr;
    }
}

void YangContext::addSearchPath(const std::string& path) {
    if (!ctx_) return;
    ly_ctx_set_searchdir(ctx_, path.c_str());
    search_paths_.push_back(path);
}

struct lys_module* YangContext::loadModuleInContext(const std::string& name,
                                                    const std::string& revision,
                                                    const char **features) {
    const char *rev = revision.empty() ? nullptr : revision.c_str();
    struct lys_module *mod = ly_ctx_load_module(ctx_, name.c_str(), rev, features);
    if (!mod) throw YangDataError(*this);
    return mod;
}

YangContext::ModuleIterator::ModuleIterator(const struct lys_module* start, uint32_t idx) : current_(start), current_idx_(idx) {}

YangSchemaModule YangContext::ModuleIterator::current() {
    if (!current_) {
        throw ModuleIteratorStopError();
    }
    return current_;
}

YangSchemaModule YangContext::ModuleIterator::next() {
    if (!current_) {
        throw ModuleIteratorStopError();
    }
    current_ = ly_ctx_get_module_iter(current_->ctx, &current_idx_);
    
    return current();
}

bool YangContext::ModuleIterator::hasNext() const noexcept {
    if (!current_) {
        throw ModuleIteratorStopError();
    }
    uint32_t temp_idx = current_idx_;
    const struct lys_module* next_mod = ly_ctx_get_module_iter(current_->ctx, &temp_idx);
    return next_mod != nullptr;
}

YangContext::ModuleIterator YangContext::GetModuleIterator(uint32_t start_offset) const noexcept {    
    lys_module* m = ly_ctx_get_module_iter(ctx_, start_offset);
    return ModuleIterator(m);
}

YangSchemaModule YangContext::GetLoadedModuleByName(const std::string& name) const noexcept {
    uint32_t idx = 0;
    for (auto it = GetModuleIterator(idx); it.hasNext(); it.next()) {
        YangSchemaModule m = it.next();
        const char *n = m.name();
        if (n && name == n) return m;
    }
    return YangSchemaModule(nullptr);
}
