#include "Yang.hpp"
#include <atf-c++.hpp>

using namespace yang;

ATF_TEST_CASE(yang_context_singleton);
ATF_TEST_CASE_HEAD(yang_context_singleton) {
  set_md_var("descr", "Yang getContext/getDefaultContext singleton behavior");
}
ATF_TEST_CASE_BODY(yang_context_singleton) {
  auto c1 = Yang::getContext({});
  ATF_REQUIRE(c1 != nullptr);

  std::shared_ptr<YangContext> c2 = Yang::getDefaultContext();
  ATF_REQUIRE(c2 != nullptr);
  ATF_REQUIRE(c1.get() == c2.get());
  ATF_REQUIRE(c2->raw() != nullptr);
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, yang_context_singleton); }
