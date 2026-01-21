#include "IetfNetworkInstance.hpp"
#include "Yang.hpp"
#include "YangContext.hpp"
#include "YangModel.hpp"
#include <atf-c++.hpp>
#include <memory>
#include <cstdio>
#include <libyang/log.h>

using namespace yang;

ATF_TEST_CASE(ietf_network_instance_roundtrip);
ATF_TEST_CASE_HEAD(ietf_network_instance_roundtrip) {
  set_md_var("descr", "IetfNetworkInstances serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_network_instance_roundtrip) {
  try {
    /* Enable libyang logging for this test (debug + store) */
    ly_set_log_clb([](LY_LOG_LEVEL level, const char *msg,
                      const char *data_path, const char *schema_path,
                      uint64_t line) {
       std::fprintf(stderr, "libyang[%d]: %s (data:%s schema:%s line:%llu)\n",
                    (int)level, msg ? msg : "", data_path ? data_path : "-",
                    schema_path ? schema_path : "-", (unsigned long long)line);
     });
     (void)ly_log_level(LY_LLDBG);
     (void)ly_log_options(LY_LOLOG | LY_LOSTORE);

     auto ctx = Yang::getDefaultContext();

     // (schema-mount callback registration removed)
     
     const std::string xml = R"(<?xml version="1.0"?>
<network-instances xmlns="urn:ietf:params:xml:ns:yang:ietf-network-instance">
  <network-instance>
    <name>Customer-A</name>
    <enabled>true</enabled>    
    <vrf-root>
        <routing xmlns="urn:ietf:params:xml:ns:yang:ietf-routing"
                xmlns:if="urn:ietf:params:xml:ns:yang:ietf-interfaces">
        <control-plane-protocols>
            <control-plane-protocol>
            <type>static</type>
            <name>static0</name>
            <description>static routes for testing</description>         
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
                <source-protocol>static</source-protocol>
                <next-hop>
                    <outgoing-interface>eth0</outgoing-interface>
                </next-hop>
                </route>
            </routes>
            </rib>
        </ribs>      
        </routing>
    </vrf-root>    
  </network-instance>
</network-instances>)";

    struct lyd_node *tree = YangModel::parseXml(*ctx, xml);
    ATF_REQUIRE(tree != nullptr);

    auto parsed = IetfNetworkInstances::deserialize(*ctx, tree);
    ATF_REQUIRE(parsed != nullptr);

    const auto &nis = parsed->getNetworkInstances();
    ATF_REQUIRE(nis.size() == 1);
    ATF_REQUIRE(nis[0].getName() == std::string("Customer-A"));
    ATF_REQUIRE(nis[0].getEnabled() == true);

    lyd_free_all(tree);
  } catch (const YangError &e) {
    ATF_FAIL(std::string("YangError exception: ") + e.what());
  } catch (const std::exception &e) {
    ATF_FAIL(std::string("std::exception: ") + e.what());
  }
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, ietf_network_instance_roundtrip); }
