#include "IetfNetworkInstance.hpp"
#include "Yang.hpp"
#include "YangContext.hpp"
#include "YangModel.hpp"
#include <atf-c++.hpp>
#include <cstdio>
#include <libyang/log.h>
#include <memory>

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

    // register ext-data callback for schema-mount handling
    ctx->registerExtDataCallback(&IetfNetworkInstances::extDataCallback,
                                 nullptr);

    // Ensure critical modules are marked implemented so yang-library/schema-mount 
    // are available, the callback requires this.
    ctx->ensureYangLibraryImplemented();
    ctx->ensureSchemaMountImplemented();

    // Test XML structure:
    // - ietf-network-instance defines mount points (vrf-root/vsi-root/vv-root) as containers
    // - ietf-routing defines a top-level 'routing' container with control-plane-protocols list
    // - The mounted data (routing config) goes directly inside vrf-root
    // - NO yang-library or schema-mounts should be in the instance data
    // - The ext-data callback provides yang-library + schema-mounts separately when requested by libyang
    const std::string xml = R"(<?xml version="1.0"?>
<network-instances xmlns="urn:ietf:params:xml:ns:yang:ietf-network-instance">
  <network-instance>
    <name>VRF-A</name>    
    <enabled>true</enabled>
    <description>Test VRF instance</description>
    <vrf-root>
      <routing xmlns="urn:ietf:params:xml:ns:yang:ietf-routing">
        <control-plane-protocols>
          <control-plane-protocol>
            <type xmlns:rt="urn:ietf:params:xml:ns:yang:ietf-routing">rt:static</type>
            <name>static-routing-vrf-a</name>
            <static-routes>
              <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ipv4-unicast-routing">
                <route>
                  <destination-prefix>10.0.0.0/24</destination-prefix>
                  <next-hop>
                    <next-hop-address>192.168.1.1</next-hop-address>
                  </next-hop>
                </route>
              </ipv4>
            </static-routes>
          </control-plane-protocol>
        </control-plane-protocols>
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
    ATF_REQUIRE(nis[0].getName() == std::string("VRF-A"));
    ATF_REQUIRE(nis[0].getEnabled() == true);

    lyd_free_all(tree);
  } catch (const YangError &e) {
    ATF_FAIL(std::string("YangError exception: ") + e.what());
  } catch (const std::exception &e) {
    ATF_FAIL(std::string("std::exception: ") + e.what());
  }
}

ATF_INIT_TEST_CASES(tcs) {
  ATF_ADD_TEST_CASE(tcs, ietf_network_instance_roundtrip);
}
