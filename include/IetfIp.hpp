#pragma once

#include "YangModel.hpp"
#include "IetfYangTypes.hpp"
#include "IetfInetTypes.hpp"

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace yang {

// C++ representation of the `ietf-ip` YANG augment for interfaces.
// Types are expressed using the typedefs in IetfYangTypes and IetfInetTypes
// to match the YANG typedefs (e.g., ipv4-address-no-zone, phys-address).
// Split the IPv4 and IPv6 containers into separate classes that match
// the `ietf-ip` augmentation: the YANG model augments `if:interface`
// with `ipv4` and `ipv6` containers (not a single `ip` wrapper).

class IetfIpv4 : public YangModel {
public:
    IetfIpv4() = default;
    ~IetfIpv4() override = default;

    bool enabled = true;
    bool forwarding = false;
    std::optional<uint16_t> mtu;

    enum class IpAddressOrigin { Other, Static, Dhcp, LinkLayer, Random };

    struct Address {
        yang::ipv4_address_no_zone ip;
        std::optional<uint8_t> prefix_length;
        std::optional<yang::dotted_quad> netmask;
        std::optional<IpAddressOrigin> origin; // operational
    };

    const std::vector<Address>& getAddresses() const noexcept { return addresses; }
    void addAddress(const Address &a) { addresses.push_back(a); }
    bool removeAddressByIp(const yang::ipv4_address_no_zone &ip);

    enum class NeighborOrigin { Other, Static, Dynamic };
    struct Neighbor {
        yang::ipv4_address_no_zone ip;
        yang::phys_address link_layer_address;
        std::optional<NeighborOrigin> origin; // operational
    };

    const std::vector<Neighbor>& getNeighbors() const noexcept { return neighbors; }
    void addNeighbor(const Neighbor &n) { neighbors.push_back(n); }
    bool removeNeighborByIp(const yang::ipv4_address_no_zone &ip);

    struct lyd_node* serialize(const YangContext &ctx) const override;
    static std::unique_ptr<IetfIpv4> deserialize(const YangContext &ctx, struct lyd_node* tree);

private:
    std::vector<Address> addresses;
    std::vector<Neighbor> neighbors;
};


class IetfIpv6 : public YangModel {
public:
    IetfIpv6() = default;
    ~IetfIpv6() override = default;

    bool enabled = true;
    bool forwarding = false;
    std::optional<uint32_t> mtu;

    enum class IpAddressOrigin { Other, Static, Dhcp, LinkLayer, Random };

    struct Address {
        yang::ipv6_address_no_zone ip;
        uint8_t prefix_length = 0; // mandatory
        std::optional<IpAddressOrigin> origin; // operational
        enum class Status { Preferred, Deprecated, Invalid, Inaccessible, Unknown, Tentative, Duplicate, Optimistic };
        std::optional<Status> status; // operational
    };

    const std::vector<Address>& getAddresses() const noexcept { return addresses; }
    void addAddress(const Address &a) { addresses.push_back(a); }
    bool removeAddressByIp(const yang::ipv6_address_no_zone &ip);

    enum class NeighborOrigin { Other, Static, Dynamic };
    struct Neighbor {
        yang::ipv6_address_no_zone ip;
        std::optional<yang::phys_address> link_layer_address;
        std::optional<NeighborOrigin> origin;
        bool is_router = false;
        std::optional<Address::Status> state;
    };

    const std::vector<Neighbor>& getNeighbors() const noexcept { return neighbors; }
    void addNeighbor(const Neighbor &n) { neighbors.push_back(n); }
    bool removeNeighborByIp(const yang::ipv6_address_no_zone &ip);

    uint32_t dup_addr_detect_transmits = 1;

    struct Autoconf {
        bool create_global_addresses = true;
        bool create_temporary_addresses = false;
        uint32_t temporary_valid_lifetime = 604800;
        uint32_t temporary_preferred_lifetime = 86400;
    } autoconf;

    struct lyd_node* serialize(const YangContext &ctx) const override;
    static std::unique_ptr<IetfIpv6> deserialize(const YangContext &ctx, struct lyd_node* tree);

private:
    std::vector<Address> addresses;
    std::vector<Neighbor> neighbors;
};

// Inline implementations for simple remove operations
inline bool IetfIpv4::removeAddressByIp(const yang::ipv4_address_no_zone &ip) {
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        if (it->ip == ip) { addresses.erase(it); return true; }
    }
    return false;
}

inline bool IetfIpv4::removeNeighborByIp(const yang::ipv4_address_no_zone &ip) {
    for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
        if (it->ip == ip) { neighbors.erase(it); return true; }
    }
    return false;
}

inline bool IetfIpv6::removeAddressByIp(const yang::ipv6_address_no_zone &ip) {
    for (auto it = addresses.begin(); it != addresses.end(); ++it) {
        if (it->ip == ip) { addresses.erase(it); return true; }
    }
    return false;
}

inline bool IetfIpv6::removeNeighborByIp(const yang::ipv6_address_no_zone &ip) {
    for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
        if (it->ip == ip) { neighbors.erase(it); return true; }
    }
    return false;
}

} // namespace yang
