#include <atf-c++.hpp>
#include <memory>
#include "YangContext.hpp"
#include "Yang.hpp"
#include "IetfIp.hpp"

using namespace yang;

ATF_TEST_CASE(ietf_ip_ipv4_roundtrip);
ATF_TEST_CASE_HEAD(ietf_ip_ipv4_roundtrip) {
    set_md_var("descr", "IetfIpv4 serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_ip_ipv4_roundtrip) {
    auto ctx = Yang::getDefaultContext();
        // Create XML for an ipv4 container and parse it
        const std::string xml = R"(<?xml version="1.0"?>
    <ipv4 xmlns="urn:ietf:params:xml:ns:yang:ietf-ip">
        <enabled>false</enabled>
        <forwarding>true</forwarding>
        <mtu>1500</mtu>
        <address>
            <ip>192.0.2.1</ip>
            <prefix-length>24</prefix-length>
        </address>
        <address>
            <ip>198.51.100.5</ip>
            <prefix-length>28</prefix-length>
        </address>
        <neighbor>
            <ip>192.0.2.2</ip>
            <link-layer-address>00:11:22:33:44:55</link-layer-address>
        </neighbor>
    </ipv4>)";

        struct lyd_node *tree = nullptr;
        LY_ERR rc = lyd_parse_data_mem(ctx->raw(), xml.c_str(), LYD_XML, 0, 0, &tree);
        ATF_REQUIRE(rc == LY_SUCCESS && tree != nullptr);

        auto parsed = IetfIpv4::deserialize(*ctx, tree);
        ATF_REQUIRE(parsed != nullptr);
        ATF_REQUIRE(parsed->getAddresses().size() == 2);
        ATF_REQUIRE(parsed->getNeighbors().size() == 1);
        ATF_REQUIRE(parsed->enabled == false);
        ATF_REQUIRE(parsed->forwarding == true);
        ATF_REQUIRE(parsed->mtu.has_value() && *parsed->mtu == 1500);

        ATF_REQUIRE(parsed->getAddresses()[0].ip == "192.0.2.1");
        ATF_REQUIRE(parsed->getAddresses()[1].ip == "198.51.100.5");

        lyd_free_all(tree);
}

ATF_INIT_TEST_CASES(tcs) {
    ATF_ADD_TEST_CASE(tcs, ietf_ip_ipv4_roundtrip);
}
