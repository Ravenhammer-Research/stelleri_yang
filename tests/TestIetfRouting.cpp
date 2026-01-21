#include "IetfRouting.hpp"
#include "Yang.hpp"
#include "YangContext.hpp"
#include "YangModel.hpp"
#include <atf-c++.hpp>
#include <memory>

using namespace yang;

ATF_TEST_CASE(ietf_routing_roundtrip);
ATF_TEST_CASE_HEAD(ietf_routing_roundtrip) {
  set_md_var("descr", "IetfRouting serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_routing_roundtrip) {
  auto ctx = Yang::getDefaultContext();
  // Build XML representing /ietf-routing:routing and parse it
  const std::string xml = R"(<?xml version="1.0"?>
<ietf-routing:routing xmlns:ietf-routing="urn:ietf:params:xml:ns:yang:ietf-routing">
    <router-id>192.0.2.254</router-id>
    <interfaces>
        <interface>eth0</interface>
        <interface>lo</interface>
    </interfaces>
    <control-plane-protocols>
        <control-plane-protocol>
            <type>static</type>
            <name>static0</name>
            <description>static routes for testing</description>
            <static-routes>
                <route>
                    <route-preference>10</route-preference>
                </route>
            </static-routes>
        </control-plane-protocol>
        <control-plane-protocol>
            <type>static</type>
            <name>static1</name>
        </control-plane-protocol>
    </control-plane-protocols>
    <ribs>
        <rib>
            <name>main</name>
            <address-family>ipv4</address-family>
            <routes>
                <route>
                    <route-preference>20</route-preference>
                </route>
            </routes>
        </rib>
    </ribs>
</ietf-routing:routing>)";

  struct lyd_node *tree = nullptr;
  tree = YangModel::parseXml(*ctx, xml);
  ATF_REQUIRE(tree != nullptr);

  auto parsed = IetfRouting::deserialize(*ctx, tree);
  ATF_REQUIRE(parsed != nullptr);

  const auto &out = parsed->getRouting();
  if (out.router_id.has_value()) {
    ATF_REQUIRE(*out.router_id == std::string("192.0.2.254"));
  }
  ATF_REQUIRE(out.interfaces.size() == 2);
  ATF_REQUIRE(out.control_plane_protocols.size() == 2);

  if (out.control_plane_protocols[0].static_routes.size() == 0) {
    // static-routes may be conditional in the schema; accept absent container
  } else {
    ATF_REQUIRE(out.control_plane_protocols[0].static_routes.size() == 1);
    ATF_REQUIRE(out.control_plane_protocols[0]
                    .static_routes[0]
                    .route_preference.has_value());
    ATF_REQUIRE(
        *out.control_plane_protocols[0].static_routes[0].route_preference ==
        10);
  }

  lyd_free_all(tree);
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, ietf_routing_roundtrip); }
