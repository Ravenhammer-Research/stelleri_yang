#include "IetfIp.hpp"
#include "Exceptions.hpp"
#include <libyang/libyang.h>

using namespace yang;

static char bool_to_str(bool v) { return v ? '1' : '0'; }

struct lyd_node* IetfIpv4::serialize(const YangContext &ctx) const {
	struct ly_ctx *c = ctx.raw();

	// create a temporary interface parent under /ietf-interfaces:interfaces
	struct lyd_node *top = nullptr;
	LY_ERR err = lyd_new_path(nullptr, c, "/ietf-interfaces:interfaces", NULL, 0, &top);
	if (err != LY_SUCCESS || !top) throw YangDataError(ctx);

	struct lyd_node *ifnode = nullptr;
	// create an interface list entry with a dummy name so the augment target exists
	err = lyd_new_path(top, c, "interface[name='__tmp_if__']", NULL, 0, &ifnode);
	if (err != LY_SUCCESS || !ifnode) throw YangDataError(ctx);

	// now create the ipv4 container as a child of the interface
	struct lyd_node *root = nullptr;
	err = lyd_new_path(ifnode, c, "ietf-ip:ipv4", NULL, 0, &root);
	if (err != LY_SUCCESS || !root) throw YangDataError(ctx);

	// enabled
	if (!enabled) {
		struct lyd_node *tmp = nullptr; (void)tmp;
		lyd_new_path(root, c, "enabled", "false", 0, &tmp);
	}
	// forwarding
	if (forwarding) {
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "forwarding", "true", 0, &tmp); }
	}
	if (mtu.has_value()) {
		std::string val = std::to_string(*mtu);
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "mtu", val.c_str(), 0, &tmp); }
	}

	// addresses
	for (const auto &a : addresses) {
		std::string ipval = a.ip;
		std::string rel = "address[ip='" + ipval + "']/ip";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel.c_str(), ipval.c_str(), 0, &tmp); }
		if (a.prefix_length.has_value()) {
			std::string pl = std::to_string(*a.prefix_length);
			std::string rel_pl = "address[ip='" + ipval + "']/prefix-length";
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_pl.c_str(), pl.c_str(), 0, &tmp); }
		}
		if (a.netmask.has_value()) {
			std::string nm = *a.netmask;
			std::string rel_nm = "address[ip='" + ipval + "']/netmask";
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_nm.c_str(), nm.c_str(), 0, &tmp); }
		}
	}

	// neighbors
	for (const auto &n : neighbors) {
		std::string ipval = n.ip;
		std::string rel_ip = "neighbor[ip='" + ipval + "']/ip";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_ip.c_str(), ipval.c_str(), 0, &tmp); }
		std::string rel_ll = "neighbor[ip='" + ipval + "']/link-layer-address";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_ll.c_str(), n.link_layer_address.c_str(), 0, &tmp); }
	}

	return root;
}

std::unique_ptr<IetfIpv4> IetfIpv4::deserialize(const YangContext &ctx, struct lyd_node* tree) {
	if (!tree) throw YangDataError(ctx);
	auto model = std::make_unique<IetfIpv4>();

	struct lyd_node *ipv4 = nullptr;
	if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "ipv4") == 0) {
		ipv4 = tree;
	} else {
		if (lyd_find_path(tree, "/ietf-ip:ipv4", 0, &ipv4) != LY_SUCCESS) ipv4 = nullptr;
	}
	if (!ipv4) throw YangDataError(ctx);

	// helper to find direct child by name
	auto find_child = [](struct lyd_node *p, const char *n) -> struct lyd_node* {
		if (!p) return nullptr;
		for (struct lyd_node *c = lyd_child(p); c; c = c->next) {
			if (c->schema && c->schema->name && strcmp(c->schema->name, n) == 0) return c;
		}
		return nullptr;
	};

	const char *v = nullptr;
	struct lyd_node *c = find_child(ipv4, "enabled");
	if (c && (v = lyd_get_value(c))) model->enabled = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);

	c = find_child(ipv4, "forwarding");
	if (c && (v = lyd_get_value(c))) model->forwarding = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);

	c = find_child(ipv4, "mtu");
	if (c && (v = lyd_get_value(c))) {
		try { model->mtu = static_cast<uint16_t>(std::stoi(v)); } catch(...) {}
	}

	// addresses (list)
	for (struct lyd_node *ch = lyd_child(ipv4); ch; ch = ch->next) {
		if (!ch->schema || !ch->schema->name) continue;
		if (strcmp(ch->schema->name, "address") == 0) {
			IetfIpv4::Address addr;
			struct lyd_node *ipn = find_child(ch, "ip");
			if (ipn && (v = lyd_get_value(ipn))) addr.ip = v;
			struct lyd_node *pl = find_child(ch, "prefix-length");
			if (pl && (v = lyd_get_value(pl))) { try { addr.prefix_length = static_cast<uint8_t>(std::stoi(v)); } catch(...) {} }
			struct lyd_node *nm = find_child(ch, "netmask");
			if (nm && (v = lyd_get_value(nm))) addr.netmask = std::string(v);
			model->addAddress(addr);
		}
		else if (strcmp(ch->schema->name, "neighbor") == 0) {
			IetfIpv4::Neighbor nb;
			struct lyd_node *ipn = find_child(ch, "ip");
			if (ipn && (v = lyd_get_value(ipn))) nb.ip = v;
			struct lyd_node *ll = find_child(ch, "link-layer-address");
			if (ll && (v = lyd_get_value(ll))) nb.link_layer_address = v;
			model->addNeighbor(nb);
		}
	}

	return model;
}

struct lyd_node* IetfIpv6::serialize(const YangContext &ctx) const {
	struct ly_ctx *c = ctx.raw();

	struct lyd_node *root = nullptr;
	LY_ERR err = lyd_new_path(nullptr, c, "/ietf-ip:ipv6", NULL, 0, &root);
	if (err != LY_SUCCESS || !root) throw YangDataError(ctx);

	if (!enabled) { struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "enabled", "false", 0, &tmp); }
	if (forwarding) { struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "forwarding", "true", 0, &tmp); }
	if (mtu.has_value()) {
		std::string val = std::to_string(*mtu);
		struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "mtu", val.c_str(), 0, &tmp);
	}

	for (const auto &a : addresses) {
		std::string ipval = a.ip;
		std::string rel_ip = "address[ip='" + ipval + "']/ip";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_ip.c_str(), ipval.c_str(), 0, &tmp); }
		std::string rel_pl = "address[ip='" + ipval + "']/prefix-length";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_pl.c_str(), std::to_string(a.prefix_length).c_str(), 0, &tmp); }
	}
	return root;
}

std::unique_ptr<IetfIpv6> IetfIpv6::deserialize(const YangContext &ctx, struct lyd_node* tree) {
	if (!tree) throw YangDataError(ctx);
	auto model = std::make_unique<IetfIpv6>();

	struct lyd_node *ipv6 = nullptr;
	if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "ipv6") == 0) {
		ipv6 = tree;
	} else {
		if (lyd_find_path(tree, "/ietf-ip:ipv6", 0, &ipv6) != LY_SUCCESS) ipv6 = nullptr;
	}
	if (!ipv6) throw YangDataError(ctx);

	auto find_child = [](struct lyd_node *p, const char *n) -> struct lyd_node* {
		if (!p) return nullptr;
		for (struct lyd_node *c = lyd_child(p); c; c = c->next) {
			if (c->schema && c->schema->name && strcmp(c->schema->name, n) == 0) return c;
		}
		return nullptr;
	};

	const char *v = nullptr;
	struct lyd_node *c = find_child(ipv6, "enabled");
	if (c && (v = lyd_get_value(c))) model->enabled = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);

	c = find_child(ipv6, "forwarding");
	if (c && (v = lyd_get_value(c))) model->forwarding = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0);

	c = find_child(ipv6, "mtu");
	if (c && (v = lyd_get_value(c))) { try { model->mtu = static_cast<uint32_t>(std::stoul(v)); } catch(...) {} }

	for (struct lyd_node *ch = lyd_child(ipv6); ch; ch = ch->next) {
		if (!ch->schema || !ch->schema->name) continue;
		if (strcmp(ch->schema->name, "address") == 0) {
			IetfIpv6::Address addr;
			struct lyd_node *ipn = find_child(ch, "ip");
			if (ipn && (v = lyd_get_value(ipn))) addr.ip = v;
			struct lyd_node *pl = find_child(ch, "prefix-length");
			if (pl && (v = lyd_get_value(pl))) { try { addr.prefix_length = static_cast<uint8_t>(std::stoi(v)); } catch(...) {} }
			model->addAddress(addr);
		}
	}

	return model;
}

