#pragma once

#include "YangModel.hpp"
#include "IetfYangTypes.hpp"

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace yang {

// C++ representation of the `ietf-routing` YANG module (RFC 8349).
// This header provides a 1:1 mapping of the primary data nodes
// used by the rest of the codebase. Serialization/deserialization
// logic is implemented in the corresponding .cpp file.
class IetfRouting : public YangModel {
public:
	IetfRouting() = default;
	~IetfRouting() override = default;

	// next-hop list entry (key = index)
	struct NextHopListEntry {
		std::string index; // key
		std::optional<std::string> outgoing_interface; // if:interface-ref
	};

	enum class SpecialNextHop { Blackhole, Unreachable, Prohibit, Receive };

	// next-hop choice (simple | special | list)
	struct NextHop {
		// simple-next-hop
		std::optional<std::string> outgoing_interface; // if:interface-ref

		// special-next-hop
		std::optional<SpecialNextHop> special_next_hop;

		// next-hop-list
		std::vector<NextHopListEntry> next_hop_list;
	};

	struct RouteMetadata {
		std::string source_protocol; // identityref base routing-protocol
		bool active = false;         // presence (empty leaf)
		std::optional<yang::date_and_time> last_updated;
	};

	using RoutePreference = std::uint32_t; // typedef route-preference

	struct Route {
		std::optional<RoutePreference> route_preference;
		std::optional<NextHop> next_hop; // container next-hop (uses next-hop-state-content)
		std::optional<RouteMetadata> metadata; // uses route-metadata
	};

	struct Rib {
		std::string name;                 // key
		std::string address_family;       // identityref from grouping address-family
		bool default_rib = true;          // if-feature multiple-ribs; config false in model
		std::vector<Route> routes;        // config false: operational routes in the RIB
		std::optional<std::string> description;
	};

	struct ControlPlaneProtocol {
		std::string type;                 // identityref base control-plane-protocol
		std::string name;                 // key (together with type)
		std::optional<std::string> description;

		// static-routes container (present when type is 'static')
		std::vector<Route> static_routes;
	};

	struct Routing {
		// uses router-id (if-feature "router-id")
		std::optional<yang::dotted_quad> router_id;

		// container interfaces (config false)
		std::vector<std::string> interfaces; // leaf-list of if:interface-ref

		std::vector<ControlPlaneProtocol> control_plane_protocols;

		std::vector<Rib> ribs;
	};

	// Accessors
	const Routing& getRouting() const noexcept { return routing_; }
	Routing& mutableRouting() noexcept { return routing_; }

	// YangModel interface
	struct lyd_node* serialize(const YangContext &ctx) const override;
	static std::unique_ptr<IetfRouting> deserialize(const YangContext &ctx, struct lyd_node* tree);

private:
	Routing routing_;
};

} // namespace yang

