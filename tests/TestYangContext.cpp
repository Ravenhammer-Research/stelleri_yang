#include "Exceptions.hpp"
#include "Yang.hpp"
#include <algorithm>
#include <atf-c++.hpp>
#include <cstdio>
#include <cstring>
#include <inttypes.h>
#include <libyang/libyang.h>
#include <libyang/log.h>

using namespace yang;
#include <vector>

// libyang log callback placed at file scope
static void libyang_log_cb(LY_LOG_LEVEL level, const char *msg,
                           const char *data_path, const char *schema_path,
                           uint64_t line) {
  std::fprintf(stderr, "libyang[%d]: %s (data:%s schema:%s line:%llu)\n",
               (int)level, msg ? msg : "", data_path ? data_path : "-",
               schema_path ? schema_path : "-", (unsigned long long)line);
}

ATF_TEST_CASE(yang_context_raw);
ATF_TEST_CASE_HEAD(yang_context_raw) {
  set_md_var("descr", "YangContext raw() returns valid libyang context");
}
ATF_TEST_CASE_BODY(yang_context_raw) {
  /* Set the callback and increase verbosity to debug to ensure all messages are
   * emitted. */
  ly_set_log_clb(libyang_log_cb);
  (void)ly_log_level(LY_LLDBG);
  /* Ensure logging is enabled and errors are stored. */
  (void)ly_log_options(LY_LOLOG | LY_LOSTORE);
  /* Enable common debug groups (only effective for debug builds). */
  (void)ly_log_dbg_groups(LY_LDGXPATH | LY_LDGDEPSETS);

  auto ctx = Yang::getContext({YangContextOption::NoYanglibrary});
  ATF_REQUIRE(ctx != nullptr);
  // Ensure the context knows where to find installed YANG modules
  const std::vector<std::string> testPaths = {
      "/usr/local/share/yang/modules/yang/standard/ietf/RFC/"};
  for (const auto &p : testPaths) {
    ctx->addSearchPath(p);
  }
  struct ly_ctx *c = ctx->raw();
  ATF_REQUIRE(c != nullptr);
  const char *msg = ly_errmsg(c);
  (void)msg;

  // Verify at least one of the default search paths was added to the context
  const auto &paths = ctx->searchPaths();
  ATF_REQUIRE(!paths.empty());
  bool any_found = false;
  for (const auto &p : testPaths) {
    if (std::find(paths.begin(), paths.end(), p) != paths.end()) {
      any_found = true;
      break;
    }
  }
  if (!any_found)
    ATF_FAIL("none of the test search paths were added to the context");

  // Try loading a known module and verify it's present in the context
  try {
    ctx->loadModuleInContext("ietf-ip", "2018-02-22", nullptr);
  } catch (YangDataError &e) {
    ATF_FAIL(e.what());
  }

  // Verify module present via our YangContext helper
  YangSchemaModule mod = ctx->GetLoadedModuleByName("ietf-ip");
  if (!mod)
    ATF_FAIL("ietf-ip module not found in libyang context");
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, yang_context_raw); }
