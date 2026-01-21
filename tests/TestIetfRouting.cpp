#include "IetfRouting.hpp"
#include "Yang.hpp"
#include "YangContext.hpp"
#include "YangModel.hpp"
#include <atf-c++.hpp>
#include <cstdio>
#include <libyang/log.h>
#include <memory>

using namespace yang;

ATF_TEST_CASE(ietf_routing_roundtrip);
ATF_TEST_CASE_HEAD(ietf_routing_roundtrip) {
  set_md_var("descr", "IetfRouting serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_routing_roundtrip) {
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
    // Build a well-formed XML `data` document that contains both the
    // `ietf-routing:routing` instance and the `ietf-interfaces:interfaces`
    // target of the leafref so libyang validation succeeds.
    const std::string xml = R"(<?xml version="1.0"?>
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

    <interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces" 
                xmlns:ianaift="urn:ietf:params:xml:ns:yang:iana-if-type" 
                xmlns:ip="urn:ietf:params:xml:ns:yang:ietf-ip">
        <interface>
            <name>eth0</name>
            <description>uplink</description>
            <type>ianaift:ethernetCsmacd</type>
            <enabled>true</enabled>
            <oper-status>up</oper-status>
            <statistics>
                <discontinuity-time>2026-01-01T00:00:00Z</discontinuity-time>
            </statistics>
            <ip:ipv4>
                    <ip:mtu>1500</ip:mtu>
                    <ip:address>
                        <ip:ip>192.0.2.1</ip:ip>
                        <ip:prefix-length>24</ip:prefix-length>
                    </ip:address>
                    <ip:address>
                        <ip:ip>198.51.100.5</ip:ip>
                        <ip:prefix-length>24</ip:prefix-length>
                    </ip:address>
            </ip:ipv4>
            <ip:ipv6>
                <ip:address>
                    <ip:ip>2001:db8::1</ip:ip>
                    <ip:prefix-length>64</ip:prefix-length>
                </ip:address>
            </ip:ipv6>
            <!-- interface-level mtu moved to per-address in test -->
        </interface>
        <interface>
            <name>lo</name>
            <description>loopback</description>
            <enabled>false</enabled>
            <type>ianaift:softwareLoopback</type>
            <oper-status>down</oper-status>
            <statistics>
                <discontinuity-time>2026-01-01T00:00:00Z</discontinuity-time>
            </statistics>
            <ip:ipv4>
                <ip:mtu>65535</ip:mtu>
                <ip:address>
                        <ip:ip>127.0.0.1</ip:ip>
                        <ip:prefix-length>8</ip:prefix-length>
                    </ip:address>
            </ip:ipv4>
        </interface>
    </interfaces>)";

    struct lyd_node *tree = nullptr;
    tree = YangModel::parseXml(*ctx, xml);
    ATF_REQUIRE(tree != nullptr);

    auto parsed = IetfRouting::deserialize(*ctx, tree);
    ATF_REQUIRE(parsed != nullptr);

    const auto &out = parsed->getRouting();

    // The test XML does not include a router-id; ensure it's absent
    ATF_REQUIRE(!out.router_id.has_value());

    // Interfaces leaf-list should contain both interfaces declared
    ATF_REQUIRE(out.interfaces.size() == 2);
    bool found_eth0 = false, found_lo = false;
    for (const auto &n : out.interfaces) {
      if (n == "eth0")
        found_eth0 = true;
      if (n == "lo")
        found_lo = true;
    }
    ATF_REQUIRE(found_eth0 && found_lo);

    // Detailed interface objects should be present when top-level
    // /ietf-interfaces:interfaces was provided
    ATF_REQUIRE(parsed->getInterfacesInfo().size() == 2);
    bool info_eth0 = false, info_lo = false;
    for (const auto &iface : parsed->getInterfacesInfo()) {
      if (iface.name == "eth0") {
        info_eth0 = true;
        if (iface.description.has_value())
          ATF_REQUIRE(*iface.description == std::string("uplink"));
      }
      if (iface.name == "lo")
        info_lo = true;
    }
    ATF_REQUIRE(info_eth0 && info_lo);

    // Control-plane-protocols per the test XML
    ATF_REQUIRE(out.control_plane_protocols.size() == 2);

    // RIBs: one RIB 'main' with a single route having preference 20
    ATF_REQUIRE(out.ribs.size() == 1);
    const auto &rib = out.ribs[0];
    ATF_REQUIRE(rib.name == std::string("main"));
    ATF_REQUIRE(rib.address_family == std::string("ipv4"));
    ATF_REQUIRE(rib.routes.size() == 1);
    ATF_REQUIRE(rib.routes[0].route_preference.has_value());
    ATF_REQUIRE(*rib.routes[0].route_preference == 20u);

    lyd_free_all(tree);
  } catch (const YangError &e) {
    ATF_FAIL(std::string("YangError exception: ") + e.what());
  } catch (const std::exception &e) {
    ATF_FAIL(std::string("std::exception: ") + e.what());
  }
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, ietf_routing_roundtrip); }
