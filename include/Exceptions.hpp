#pragma once

#include "YangContext.hpp"
#include <exception>
#include <libyang/libyang.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>
#include <execinfo.h>
#include <cxxabi.h>
#include <algorithm>
#include <iostream>
#include <cstdlib>

namespace yang {

  class YangError : public std::exception {
  public:
    // Public wrapper so external code (e.g. terminate handler) can reuse the
    // existing protected captureStackTrace implementation.
    static std::string captureStackTracePublic(size_t max_frames = 64) {
      return captureStackTrace(max_frames);
    }

    // capture stack trace into string at construction
    YangError() : trace_(captureStackTrace()) {}

    // what() uses the captured ly_err_item message if available
    const char *what() const noexcept override {
      static std::string buf;
      buf = std::string("[No YANG ctx Available]: ") + getError() + "\nStack trace:\n" + getStackTraceString();
      return buf.c_str();
    }

    // Accessors for ly_err_item fields (lowercase)
    virtual std::string getError() const {
      return std::string(ly_last_logmsg());
    }

    // Two path accessors as requested
    virtual std::string getDataPath() const { return std::string(); }
    virtual std::string getSchemaPath() const { return std::string(); }

    virtual int getLine() const { return -1; }

    virtual std::string getAppTag() const { return std::string(); }

    virtual std::string getModuleName() const { return std::string(); }

    virtual std::string getExpr() const { return std::string(); }

    // Provide stack-trace stringification
    virtual std::string getStackTraceString() const {
      return trace_;
    }

  protected:
    const struct ly_err_item *error_ = nullptr; // only class property allowed
    // Capture libyang error using provided context pointer; sets error_ and
    // message_ and returns textual message if available.
    const ly_err_item *captureError(const YangContext *ctx) {
      return ly_err_last(ctx->raw());
    }

    // Capture stack trace via backtrace/backtrace_symbols and attempt to demangle.
    static std::string captureStackTrace(size_t max_frames = 64) {
      std::string out;
      std::vector<void*> frames(max_frames);
      int n = backtrace(frames.data(), static_cast<int>(max_frames));
      if (n <= 0)
        return out;
      char **symbols = backtrace_symbols(frames.data(), n);
      if (!symbols)
        return out;
      for (int i = 0; i < n; ++i) {
        std::string sym(symbols[i]);
        // attempt to demangle function name between '(' and '+' if present
        size_t begin = sym.find('(');
        size_t plus = sym.find('+', begin == std::string::npos ? 0 : begin);
        if (begin != std::string::npos && plus != std::string::npos && plus > begin+1) {
          std::string mangled = sym.substr(begin+1, plus - (begin+1));
          int status = 0;
          char *dem = abi::__cxa_demangle(mangled.c_str(), nullptr, nullptr, &status);
          if (dem && status == 0) {
            sym.replace(begin+1, plus - (begin+1), dem);
          }
          free(dem);
        }
        out += sym;
        out += '\n';
      }
      free(symbols);
      return out;
    }

    std::string trace_;
  };

  // YangDataError: constructed with a YangContext and calls CaptureError(&ctx)
  class YangDataError : public YangError {
  public:
    explicit YangDataError(const YangContext &ctx) : ctx_(&ctx) {
      error_ = captureError(&ctx);
    }

    // Provide richer overrides that read from the captured ly_err_item chain
    const char *what() const noexcept override {
      if (what_buf_.empty()) {
        std::string mod = getModuleName();
        std::string err = getError();
        std::string dpath = getDataPath();
        std::string spath = getSchemaPath();
        int line = getLine();
        std::string apptag = getAppTag();
        std::string expr = getExpr();

        std::ostringstream ss;
        if (!mod.empty())
          ss << "[" << mod << "] ";
        if (!err.empty())
          ss << err;
        if (!dpath.empty())
          ss << "\nData path: " << dpath;
        if (!spath.empty())
          ss << "\nSchema path: " << spath;
        if (line >= 0)
          ss << "\nLine: " << std::to_string(line);
        if (!apptag.empty())
          ss << "\nApp tag: " << apptag;
        if (!expr.empty())
          ss << "\nExpr: " << expr;

        // append stack trace
        ss << "\nStack trace:\n" << getStackTraceString();

        what_buf_ = ss.str();
      }
      return what_buf_.c_str();
    }

  private:
    mutable std::string what_buf_;

    std::string getError() const override {
      if (error_ && error_->msg)
        return std::string(error_->msg);
      return YangError::getError();
    }

    std::string getDataPath() const override {
      if (error_ && error_->data_path)
        return std::string(error_->data_path);
      return YangError::getDataPath();
    }

    std::string getSchemaPath() const override {
      if (error_ && error_->schema_path)
        return std::string(error_->schema_path);
      return YangError::getSchemaPath();
    }

    int getLine() const override {
      if (error_)
        return static_cast<int>(error_->line);
      return YangError::getLine();
    }

    std::string getAppTag() const override {
      if (error_ && error_->apptag)
        return std::string(error_->apptag);
      return YangError::getAppTag();
    }

    YangDataError(const YangDataError &) = default;
    YangDataError &operator=(const YangDataError &) = default;
    const YangContext *ctx_ = nullptr;
  };

  // Exception thrown when module iterator cannot continue (no more modules).
  class ModuleIteratorStopError : public std::runtime_error {
  public:
    ModuleIteratorStopError() : std::runtime_error(nullptr) {}
  };

  // Terminate handler that dumps a stack trace (uses inline to avoid ODR)
  inline void yang_terminate_handler() {
    std::cerr << "Unhandled exception - terminate called. Stack trace:\n"
              << YangError::captureStackTracePublic(64) << std::endl;
    std::abort();
  }

  // Install the terminate handler at static init time.
  inline struct YangTerminateInstaller {
    YangTerminateInstaller() { std::set_terminate(yang_terminate_handler); }
  } yangTerminateInstaller;

} // namespace yang
