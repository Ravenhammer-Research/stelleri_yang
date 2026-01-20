#pragma once

#include "YangModel.hpp"
#include "IetfYangTypes.hpp"
#include "IanaIfType.hpp"

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace yang {

// C++ representation of the `ietf-interfaces` YANG module.
// This header provides a 1:1 mapping of the YANG data nodes
// used by the rest of the codebase. Serialization/deserialization
// logic is implemented in the corresponding .cpp file.
class IetfInterfaces : public YangModel {
public:
    IetfInterfaces() = default;
    ~IetfInterfaces() override = default;

    struct IetfInterfaceStatistics {
        std::optional<yang::date_and_time> discontinuity_time;

        std::optional<yang::counter64> in_octets;
        std::optional<yang::counter64> in_unicast_pkts;
        std::optional<yang::counter64> in_broadcast_pkts;
        std::optional<yang::counter64> in_multicast_pkts;
        std::optional<yang::counter32> in_discards;
        std::optional<yang::counter32> in_errors;
        std::optional<yang::counter32> in_unknown_protos;

        std::optional<yang::counter64> out_octets;
        std::optional<yang::counter64> out_unicast_pkts;
        std::optional<yang::counter64> out_broadcast_pkts;
        std::optional<yang::counter64> out_multicast_pkts;
        std::optional<yang::counter32> out_discards;
        std::optional<yang::counter32> out_errors;
    };

    // Minimal structs representing the `ietf-ip` augmentation
    // that used to live in `IetfIp.hpp`. These are intentionally
    // lightweight placeholders so the interfaces module can
    // refer to `ipv4`/`ipv6` containers without the separate file.
    struct IetfIpv4 {
        std::vector<std::string> address;
    };

    struct IetfIpv6 {
        std::vector<std::string> address;
    };

    struct IetfInterface {
        // key
        std::string name;

        // configuration (container: interfaces)
        std::optional<std::string> description;
        std::optional<yang::IanaIfType> type; // identityref -> IanaIfType enum
        bool enabled = true;
        std::optional<uint32_t> mtu;

        enum class LinkUpDownTrap { Enabled, Disabled };
        std::optional<LinkUpDownTrap> link_up_down_trap_enable;

        // operational (container: interfaces-state)
        std::optional<std::string> admin_status;
        std::optional<std::string> oper_status;
        std::optional<yang::date_and_time> last_change; // yang:date-and-time
        std::optional<int32_t> if_index;
        std::optional<yang::phys_address> phys_address;
        std::vector<std::string> higher_layer_if;
        std::vector<std::string> lower_layer_if;
        std::optional<yang::gauge64> speed;
        std::optional<IetfInterfaceStatistics> statistics;
        // Augmentation: `ietf-ip` adds `ipv4` and `ipv6` containers
        // directly under the interface. Represent those as optionals.
        std::optional<IetfIpv4> ipv4;
        std::optional<IetfIpv6> ipv6;
    };

    // Accessors
    const std::vector<IetfInterface>& getInterfaces() const noexcept { return ifs_; }
    void addInterface(const IetfInterface &i) { ifs_.push_back(i); }
    bool removeInterfaceByName(const std::string &name);

    // YangModel interface
    struct lyd_node* serialize(const YangContext &ctx) const override;
    static std::unique_ptr<IetfInterfaces> deserialize(const YangContext &ctx, struct lyd_node* tree);

private:
    std::vector<IetfInterface> ifs_;
};

// Inline simple helper implementations
inline bool IetfInterfaces::removeInterfaceByName(const std::string &name) {
    for (auto it = ifs_.begin(); it != ifs_.end(); ++it) {
        if (it->name == name) { ifs_.erase(it); return true; }
    }
    return false;
}

} // namespace yang
