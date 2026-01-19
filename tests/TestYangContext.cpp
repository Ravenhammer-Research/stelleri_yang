#include <atf-c++.hpp>
#include "Yang.hpp"
#include <libyang/libyang.h>

using namespace yang;

ATF_TEST_CASE(yang_context_raw);
ATF_TEST_CASE_HEAD(yang_context_raw) {
    set_md_var("descr", "YangContext raw() returns valid libyang context");
}
ATF_TEST_CASE_BODY(yang_context_raw) {
    auto ctx = Yang::getDefaultContext();
    ATF_REQUIRE(ctx != nullptr);
    struct ly_ctx *c = ctx->raw();
    ATF_REQUIRE(c != nullptr);
    const char *msg = ly_errmsg(c);
    (void)msg;
}

ATF_INIT_TEST_CASES(tcs) {
    ATF_ADD_TEST_CASE(tcs, yang_context_raw);
}
