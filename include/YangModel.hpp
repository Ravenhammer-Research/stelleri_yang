#pragma once

#include "YangContext.hpp"
#include "Exceptions.hpp"
#include <libyang/libyang.h>
#include <libyang/tree_data.h>
#include <memory>
#include <stdexcept>

namespace yang {

// Base class for data models that can be serialized/deserialized
// to/from libyang data trees.
class YangModel {
public:
    YangModel() = default;
    virtual ~YangModel() = default;

    // Serialize this model into a libyang data tree. The returned pointer
    // is a `struct lyd_node *` representing the root of the produced tree.
    // Ownership semantics follow libyang conventions (caller is responsible
    // for freeing the tree using libyang APIs).
    // Override this in derived classes.
    virtual struct lyd_node* serialize(const YangContext &ctx) const = 0;

    // Static factory to create a model instance from a libyang data tree.
    // NOTE: static methods cannot be virtual in C++; derived classes should
    // provide their own `deserialize(const YangContext&, struct lyd_node*)`
    // static method which returns a `std::unique_ptr<Derived>`.
    // The base implementation simply signals that it's not implemented.
    static std::unique_ptr<YangModel> deserialize(const YangContext &/*ctx*/, struct lyd_node* /*tree*/)
    {
        throw std::logic_error("deserialize not implemented for base YangModel");
    }

    // Convenience helper: parse an XML fragment into a libyang data tree using
    // the provided `YangContext`. On success returns the created `lyd_node*`.
    // On failure throws `YangDataError` constructed with `ctx`.
    static struct lyd_node* parseXml(const YangContext &ctx, const std::string &xml, uint32_t parse_flags = 0) {
        struct lyd_node *tree = nullptr;
        LY_ERR rc = lyd_parse_data_mem(ctx.raw(), xml.c_str(), LYD_XML, 0, parse_flags, &tree);
        if (rc != LY_SUCCESS || tree == nullptr) {
            throw YangDataError(ctx);
        }
        return tree;
    }
};

} // namespace yang
