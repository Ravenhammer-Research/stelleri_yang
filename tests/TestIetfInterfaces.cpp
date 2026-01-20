#include <atf-c++.hpp>
#include <memory>
#include "YangContext.hpp"
#include "Yang.hpp"
#include "IetfInterfaces.hpp"
#include "YangModel.hpp"

using namespace yang;

ATF_TEST_CASE(ietf_interfaces_roundtrip);
ATF_TEST_CASE_HEAD(ietf_interfaces_roundtrip) {
    set_md_var("descr", "IetfInterfaces serialize/deserialize roundtrip");
}
ATF_TEST_CASE_BODY(ietf_interfaces_roundtrip) {
    auto ctx = Yang::getDefaultContext();
        // Build XML for interfaces and parse it with libyang
        const std::string xml = R"(<?xml version="1.0"?>
<ietf-interfaces:interfaces xmlns:ietf-interfaces="urn:ietf:params:xml:ns:yang:ietf-interfaces" xmlns:iana-if-type="urn:ietf:params:xml:ns:yang:iana-if-type">
    <ietf-interfaces:interface>
        <ietf-interfaces:name>eth0</ietf-interfaces:name>
        <ietf-interfaces:description>uplink</ietf-interfaces:description>
        <ietf-interfaces:type>iana-if-type:ethernetCsmacd</ietf-interfaces:type>
        <ietf-interfaces:enabled>true</ietf-interfaces:enabled>
        <ietf-interfaces:mtu>1500</ietf-interfaces:mtu>
    </ietf-interfaces:interface>
    <ietf-interfaces:interface>
        <ietf-interfaces:name>lo</ietf-interfaces:name>
        <ietf-interfaces:description>loopback</ietf-interfaces:description>
        <ietf-interfaces:enabled>false</ietf-interfaces:enabled>
        <ietf-interfaces:mtu>65536</ietf-interfaces:mtu>
    </ietf-interfaces:interface>
</ietf-interfaces:interfaces>)";

        struct lyd_node *tree = nullptr;
        tree = YangModel::parseXml(*ctx, xml);
        ATF_REQUIRE(tree != nullptr);

        std::unique_ptr<IetfInterfaces> parsed = IetfInterfaces::deserialize(*ctx, tree);
        ATF_REQUIRE(parsed != nullptr);
        ATF_REQUIRE(parsed->getInterfaces().size() == 2);

        const auto &out = parsed->getInterfaces();
        ATF_REQUIRE(out[0].name == "eth0");
        ATF_REQUIRE(out[0].description.has_value() && out[0].description->find("uplink") != std::string::npos);
        ATF_REQUIRE(out[0].enabled == true);
        ATF_REQUIRE(out[0].type.has_value() && *out[0].type == yang::IanaIfType::ethernetCsmacd);

        ATF_REQUIRE(out[1].name == "lo");
        ATF_REQUIRE(out[1].enabled == false);

        lyd_free_all(tree);
}

ATF_INIT_TEST_CASES(tcs) {
    ATF_ADD_TEST_CASE(tcs, ietf_interfaces_roundtrip);
}
