#pragma once

#include <cstdint>
#include <string>

namespace yang {

// Typedefs mapping YANG `ietf-inet-types` to nearest C++ storage types.

enum class IpVersion : uint8_t { Unknown = 0, IPv4 = 1, IPv6 = 2 };

using dscp = std::uint8_t;              // YANG: dscp (0..63)
using ipv6_flow_label = std::uint32_t;  // YANG: ipv6-flow-label (0..1048575)

using port_number = std::uint16_t;      // YANG: port-number (0..65535)
using as_number = std::uint32_t;        // YANG: as-number

using ipv4_address = std::string;       // YANG: ipv4-address (string)
using ipv6_address = std::string;       // YANG: ipv6-address (string)

using ipv4_address_no_zone = std::string; // YANG: ipv4-address-no-zone
using ipv6_address_no_zone = std::string; // YANG: ipv6-address-no-zone

using ipv4_prefix = std::string;        // YANG: ipv4-prefix
using ipv6_prefix = std::string;        // YANG: ipv6-prefix

using domain_name = std::string;        // YANG: domain-name
using uri = std::string;                // YANG: uri

// Represent a version-neutral IP address (union of IPv4/IPv6).
struct IpAddress {
	enum class Type { IPv4, IPv6 } type;
	std::string address; // textual representation including optional zone when present
};

// Represent an IP prefix (ipv4-prefix or ipv6-prefix).
struct IpPrefix {
	enum class Type { IPv4, IPv6 } type;
	std::string prefix; // textual representation "address/len"
};

// Host can be either an IP address or a domain-name.
struct Host {
	enum class Type { IP, Domain } type;
	std::string value; // if IP: textual address, else domain-name
};

// For convenience, keep the union-like aliases as strings where appropriate.
using ip_address = IpAddress;           // YANG: ip-address
using ip_address_no_zone = std::string; // YANG: ip-address-no-zone
using ip_prefix = IpPrefix;             // YANG: ip-prefix
using host = Host;                      // YANG: host

} // namespace yang



