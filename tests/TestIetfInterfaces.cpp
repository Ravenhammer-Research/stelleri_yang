#include "IetfInterfaces.hpp"
#include "Yang.hpp"
#include "YangContext.hpp"
#include "YangModel.hpp"
#include <atf-c++.hpp>
#include <cstdio>
#include <libyang/log.h>
#include <memory>

using namespace yang;

// libyang log callback placed at file scope (same as TestYangContext.cpp)
static void libyang_log_cb(LY_LOG_LEVEL level, const char *msg,
                           const char *data_path, const char *schema_path,
                           uint64_t line) {
  std::fprintf(stderr, "libyang[%d]: %s (data:%s schema:%s line:%llu)\n",
               (int)level, msg ? msg : "", data_path ? data_path : "-",
               schema_path ? schema_path : "-", (unsigned long long)line);
}

ATF_TEST_CASE(ietf_interfaces_roundtrip);
ATF_TEST_CASE_HEAD(ietf_interfaces_roundtrip) {
  set_md_var("descr", "IetfInterfaces serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_interfaces_roundtrip) {
  try {
    /* Enable libyang logging for this test (debug + store) */
    ly_set_log_clb(libyang_log_cb);
    (void)ly_log_level(LY_LLDBG);
    (void)ly_log_options(LY_LOLOG | LY_LOSTORE);
    //(void)ly_log_dbg_groups(LY_LDGXPATH | LY_LDGDEPSETS);

    auto ctx = Yang::getDefaultContext();
    // Build XML for interfaces and parse it with libyang
    const std::string xml = R"(<?xml version="1.0"?>
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

    std::unique_ptr<IetfInterfaces> parsed =
        IetfInterfaces::deserialize(*ctx, tree);
    ATF_REQUIRE(parsed != nullptr);
    ATF_REQUIRE(parsed->getInterfaces().size() == 2);

    const auto &out = parsed->getInterfaces();
    ATF_REQUIRE(out[0].name == "eth0");
    ATF_REQUIRE(out[0].description.has_value());
    ATF_REQUIRE(out[0].description->find("uplink") != std::string::npos);
    ATF_REQUIRE(out[0].enabled == true);
    ATF_REQUIRE(out[0].type.has_value());
    ATF_REQUIRE(*out[0].type == yang::IanaIfType::ethernetCsmacd);
    // verify addresses, statistics and operational state were parsed
    ATF_REQUIRE(out[0].ipv4.has_value());
    ATF_REQUIRE(out[0].ipv4->address.size() == 2);
    ATF_REQUIRE(out[0].ipv4->address[0].address == "192.0.2.1/24");
    ATF_REQUIRE(out[0].ipv4->address[1].address == "198.51.100.5/24");
    ATF_REQUIRE(out[0].ipv6.has_value());
    ATF_REQUIRE(out[0].ipv6->address.size() == 1);
    ATF_REQUIRE(out[0].ipv6->address[0].address == "2001:db8::1/64");
    ATF_REQUIRE(out[0].statistics.has_value());
    ATF_REQUIRE(out[0].statistics->discontinuity_time.has_value());
    ATF_REQUIRE(out[0].statistics->discontinuity_time ==
                "2026-01-01T00:00:00Z");
    ATF_REQUIRE(out[0].oper_status.has_value());
    ATF_REQUIRE(out[0].oper_status == "up");
    ATF_REQUIRE(out[0].ipv4->mtu.has_value());
    ATF_REQUIRE(*out[0].ipv4->mtu == 1500);

    ATF_REQUIRE(out[1].name == "lo");
    ATF_REQUIRE(out[1].enabled == false);
    ATF_REQUIRE(out[1].type.has_value());
    ATF_REQUIRE(*out[1].type == yang::IanaIfType::softwareLoopback);
    ATF_REQUIRE(out[1].ipv4.has_value());
    ATF_REQUIRE(out[1].ipv4->address.size() == 1);
    ATF_REQUIRE(out[1].ipv4->address[0].address == "127.0.0.1/8");
    ATF_REQUIRE(out[1].statistics.has_value());
    ATF_REQUIRE(out[1].statistics->discontinuity_time.has_value());
    ATF_REQUIRE(out[1].statistics->discontinuity_time ==
                "2026-01-01T00:00:00Z");
    ATF_REQUIRE(out[1].oper_status.has_value());
    ATF_REQUIRE(out[1].oper_status == "down");
    ATF_REQUIRE(out[1].ipv4->mtu.has_value());
    ATF_REQUIRE(*out[1].ipv4->mtu == 65535);

    lyd_free_all(tree);
  } catch (const std::exception &e) {
    ATF_FAIL(std::string("Exception during test: ") + e.what());
  }
}

ATF_INIT_TEST_CASES(tcs) { ATF_ADD_TEST_CASE(tcs, ietf_interfaces_roundtrip); }
