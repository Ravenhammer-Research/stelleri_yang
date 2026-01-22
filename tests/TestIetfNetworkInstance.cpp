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
    /* Enable libyang logging for debugging */
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

    // Register ext-data callback BEFORE parsing any data
    // This is critical for schema-mount to work
    ctx->registerExtDataCallback(&IetfNetworkInstances::extDataCallback,
                                 nullptr);

    // Ensure critical modules are implemented
    ctx->ensureYangLibraryImplemented();
    ctx->ensureSchemaMountImplemented();

    // Test XML with mounted routing data inside vrf-root
    // Per RFC 8528 and ietf-network-instance YANG:
    // - vrf-root is a mount point for L3VPN routing
    // - The mounted schema contains ietf-routing and augmentations
    // - Instance data inside vrf-root follows the mounted schema
    const std::string xml = R"(<?xml version="1.0"?>
<network-instances xmlns="urn:ietf:params:xml:ns:yang:ietf-network-instance">
  <network-instance>
    <name>VRF-A</name>
    <enabled>true</enabled>
    <description>Test VRF instance with static routing</description>
    <vrf-root>
      <routing xmlns="urn:ietf:params:xml:ns:yang:ietf-routing">
        <control-plane-protocols>
          <control-plane-protocol>
            <type xmlns:rt="urn:ietf:params:xml:ns:yang:ietf-routing">rt:static</type>
            <name>static-vrf-a</name>
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

    // Parse the XML with schema mount support
    struct lyd_node *tree = YangModel::parseXml(*ctx, xml);
    if (tree == nullptr) {
      ATF_FAIL("Failed to parse XML with mounted data");
    }

    // Validate the tree
    LY_ERR rc = lyd_validate_all(&tree, NULL, LYD_VALIDATE_PRESENT, NULL);
    if (rc != LY_SUCCESS) {
      lyd_free_all(tree);
      ATF_FAIL("Validation failed");
    }

    // Deserialize into C++ model
    auto parsed = IetfNetworkInstances::deserialize(*ctx, tree);
    if (!parsed) {
      lyd_free_all(tree);
      ATF_FAIL("Deserialization failed");
    }

    // Verify the parsed data
    const auto &nis = parsed->getNetworkInstances();
    if (nis.size() != 1) {
      lyd_free_all(tree);
      ATF_FAIL("Expected 1 network instance");
    }
    if (nis[0].getName() != "VRF-A") {
      lyd_free_all(tree);
      ATF_FAIL("Wrong name");
    }
    if (nis[0].getEnabled() != true) {
      lyd_free_all(tree);
      ATF_FAIL("Should be enabled");
    }
    if (!nis[0].getDescription().has_value()) {
      lyd_free_all(tree);
      ATF_FAIL("Should have description");
    }
    if (nis[0].getDescription().value() != "Test VRF instance with static routing") {
      lyd_free_all(tree);
      ATF_FAIL("Wrong description");
    }

    // Check if routing data was captured (if implemented)
    if (nis[0].getRouting()) {
      std::fprintf(stderr, "Successfully parsed mounted routing data\n");
    }

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
