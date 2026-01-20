#include "Yang.hpp"
#include "YangContext.hpp"

#include <memory>
#include <vector>
#include <filesystem>
#include "Exceptions.hpp"
#include <libyang/libyang.h>
#include <string>
#include <stdexcept>

using namespace yang;

std::shared_ptr<YangContext> Yang::getContext(std::initializer_list<YangContextOption> opts) {
    unsigned flags = 0u;
    for (auto o : opts) flags |= static_cast<unsigned>(o);

    static std::shared_ptr<YangContext> instance;
    if (!instance) {
        instance = std::make_shared<YangContext>(flags);
    }
    return instance;
}

std::shared_ptr<YangContext> Yang::getDefaultContext() {
    static bool initialized = false;
    // Create a context that does not auto-initialize ietf-yang-library
    auto instance = getContext({YangContextOption::NoYanglibrary});
    if (!instance) throw std::runtime_error("failed to create YangContext");
    if (initialized) return instance;

    const auto &candidates = yang::kDefaultSearchPaths;
    size_t added = 0;
    for (const auto &p : candidates) {
        std::filesystem::path pp{p};
            if (std::filesystem::exists(pp) && std::filesystem::is_directory(pp)) {
            instance->addSearchPath(pp.string());
            ++added;
        }
    }

    if (added == 0) {
        throw yang::YangDataError(*instance);
    }

    struct ly_ctx *c = static_cast<struct ly_ctx *>(instance->raw());
    if (!c) {
        throw yang::YangDataError(*instance);
    }

    for (const auto &pr : yang::kDefaultModules) {
        const std::string &name = std::get<0>(pr);
        const std::string &rev  = std::get<1>(pr);
        const char **features = std::get<2>(pr); // pointer-to-array (nullptr for now)
        // delegate module loading to the context helper (throws on failure)
        instance->loadModuleInContext(name, rev, features);
    }

    initialized = true;
    return instance;
}
