#pragma once

#include <stdexcept>
#include <string>
#include <exception>
#include <vector>
#include <sstream>
#include <unordered_set>
#include "YangContext.hpp"
#include <libyang/libyang.h>

namespace yang {

class YangError : public std::exception {
public:
    YangError() {
    }

    // what() uses the captured ly_err_item message if available
    const char* what() const noexcept override {
        static std::string buf;
        buf = std::string("[No YANG ctx Available]: ") + getError();
        return buf.c_str();
    }

    // Accessors for ly_err_item fields (lowercase)
    virtual std::string getError() const { return std::string(ly_last_logmsg()); }

    // Two path accessors as requested
    virtual std::string getDataPath() const { return std::string(); }
    virtual std::string getSchemaPath() const { return std::string(); }

    virtual int getLine() const { return -1; }

    virtual std::string getAppTag() const { return std::string(); }

    virtual std::string getModuleName() const { return std::string(); }

    virtual std::string getExpr() const { return std::string(); }

protected:
    const struct ly_err_item* error_ = nullptr; // only class property allowed
    // Capture libyang error using provided context pointer; sets error_ and message_ and returns textual message if available.
    const ly_err_item* captureError(const YangContext* ctx) {
        return ly_err_last(ctx->raw());
    }
};

// YangDataError: constructed with a YangContext and calls CaptureError(&ctx)
class YangDataError : public YangError {
public:

    explicit YangDataError(const YangContext& ctx) {
        error_ = captureError(&ctx);
    }

    // Provide richer overrides that read from the captured ly_err_item chain
    const char* what() const noexcept override {
        if (what_buf_.empty()) {
            std::string mod = getModuleName();
            std::string err = getError();
            std::string dpath = getDataPath();
            std::string spath = getSchemaPath();
            int line = getLine();
            std::string apptag = getAppTag();
            std::string expr = getExpr();

            std::ostringstream ss;
            if (!mod.empty()) ss << "[" << mod << "] ";
            if (!err.empty()) ss << err;
            if (!dpath.empty()) ss << "\nData path: " << dpath;
            if (!spath.empty()) ss << "\nSchema path: " << spath;
            if (line >= 0) ss << "\nLine: " << line;
            if (!apptag.empty()) ss << "\nApp tag: " << apptag;
            if (!expr.empty()) ss << "\nExpr: " << expr;

            what_buf_ = ss.str();
        }
        return what_buf_.c_str();
    }

private:
    mutable std::string what_buf_;

    std::string getError() const override {
        if (error_ && error_->msg) return std::string(error_->msg);
        return YangError::getError();
    }

    std::string getDataPath() const override {
        if (error_ && error_->data_path) return std::string(error_->data_path);
        return YangError::getDataPath();
    }

    std::string getSchemaPath() const override {
        if (error_ && error_->schema_path) return std::string(error_->schema_path);
        return YangError::getSchemaPath();
    }

    int getLine() const override {
        if (error_) return static_cast<int>(error_->line);
        return YangError::getLine();
    }

    std::string getAppTag() const override {
        if (error_ && error_->apptag) return std::string(error_->apptag);
        return YangError::getAppTag();
    }

    YangDataError(const YangDataError&) = default;
    YangDataError& operator=(const YangDataError&) = default;
};

} // namespace yang
