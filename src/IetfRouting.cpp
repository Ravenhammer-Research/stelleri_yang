#include "IetfRouting.hpp"
#include "Exceptions.hpp"
#include <libyang/libyang.h>

using namespace yang;

struct lyd_node* IetfRouting::serialize(const YangContext &ctx) const {
	struct ly_ctx *c = ctx.raw();

	struct lyd_node *root = nullptr;
	LY_ERR err = lyd_new_path(nullptr, c, "/ietf-routing:routing", NULL, 0, &root);
	if (err != LY_SUCCESS || !root) throw YangDataError(ctx);

	// router-id
	if (routing_.router_id.has_value()) {
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "router-id", routing_.router_id->c_str(), 0, &tmp); }
	}

	// interfaces leaf-list
	for (const auto &ifname : routing_.interfaces) {
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, "interfaces/interface", ifname.c_str(), 0, &tmp); }
	}

	// control-plane-protocol entries (type + name + description)
	for (const auto &cpp : routing_.control_plane_protocols) {
		std::string pred = "control-plane-protocols/control-plane-protocol[type='" + cpp.type + "'][name='" + cpp.name + "']";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/type").c_str(), cpp.type.c_str(), 0, &tmp); }
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/name").c_str(), cpp.name.c_str(), 0, &tmp); }
		if (cpp.description.has_value()) {
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/description").c_str(), cpp.description->c_str(), 0, &tmp); }
		}
		// static-routes: create minimal entries
		for (const auto &r : cpp.static_routes) {
			if (r.route_preference.has_value()) {
				{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/static-routes/route/route-preference").c_str(), std::to_string(*r.route_preference).c_str(), 0, &tmp); }
			}
		}
	}

	// ribs
	for (const auto &rib : routing_.ribs) {
		std::string pred = "ribs/rib[name='" + rib.name + "']";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/name").c_str(), rib.name.c_str(), 0, &tmp); }
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/address-family").c_str(), rib.address_family.c_str(), 0, &tmp); }
		if (rib.description.has_value()) {
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/description").c_str(), rib.description->c_str(), 0, &tmp); }
		}
		for (const auto &r : rib.routes) {
			if (r.route_preference.has_value()) {
				{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, (pred + "/routes/route/route-preference").c_str(), std::to_string(*r.route_preference).c_str(), 0, &tmp); }
			}
		}
	}

	return root;
}

std::unique_ptr<IetfRouting> IetfRouting::deserialize(const YangContext &ctx, struct lyd_node* tree) {
	if (!tree) throw YangDataError(ctx);
	auto model = std::make_unique<IetfRouting>();

	struct lyd_node *rt = nullptr;
	if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "routing") == 0) {
		rt = tree;
	} else {
		if (lyd_find_path(tree, "/ietf-routing:routing", 0, &rt) != LY_SUCCESS) rt = nullptr;
	}
	if (!rt) throw YangDataError(ctx);

	auto find_child = [](struct lyd_node *p, const char *n) -> struct lyd_node* {
		if (!p) return nullptr;
		for (struct lyd_node *c = lyd_child(p); c; c = c->next) {
			if (c->schema && c->schema->name && strcmp(c->schema->name, n) == 0) return c;
		}
		return nullptr;
	};

	const char *v = nullptr;
	struct lyd_node *c = find_child(rt, "router-id");
	if (c && (v = lyd_get_value(c))) model->mutableRouting().router_id = std::string(v);
	else {
		struct lyd_node *abs_c = nullptr;
		if (lyd_find_path(tree, "/ietf-routing:routing/router-id", 0, &abs_c) == LY_SUCCESS && abs_c && (v = lyd_get_value(abs_c))) {
			model->mutableRouting().router_id = std::string(v);
		}
	}

	// interfaces leaf-list
	struct lyd_node *ifs = find_child(rt, "interfaces");
	if (ifs) {
		for (struct lyd_node *ch = lyd_child(ifs); ch; ch = ch->next) {
			if (!ch->schema || !ch->schema->name) continue;
			if (strcmp(ch->schema->name, "interface") == 0) {
				if ((v = lyd_get_value(ch))) model->mutableRouting().interfaces.push_back(std::string(v));
			}
		}
	}

	// control-plane-protocols
	struct lyd_node *cpps = find_child(rt, "control-plane-protocols");
	if (cpps) {
		for (struct lyd_node *entry = lyd_child(cpps); entry; entry = entry->next) {
			if (!entry->schema || !entry->schema->name) continue;
			if (strcmp(entry->schema->name, "control-plane-protocol") != 0) continue;
			IetfRouting::ControlPlaneProtocol cp;
			struct lyd_node *t = find_child(entry, "type"); if (t && (v = lyd_get_value(t))) {
				std::string sv(v);
				auto pos = sv.find(':');
				if (pos != std::string::npos) sv = sv.substr(pos + 1);
				cp.type = sv;
			}
			struct lyd_node *nnode = find_child(entry, "name"); if (nnode && (v = lyd_get_value(nnode))) cp.name = v;
			struct lyd_node *d = find_child(entry, "description"); if (d && (v = lyd_get_value(d))) cp.description = std::string(v);

			// static-routes (simple parsing of route-preference)
			struct lyd_node *sr = find_child(entry, "static-routes");
			if (sr) {
				for (struct lyd_node *r = lyd_child(sr); r; r = r->next) {
					if (!r->schema || !r->schema->name) continue;
					if (strcmp(r->schema->name, "route") != 0) continue;
					IetfRouting::Route route;
					struct lyd_node *rp = find_child(r, "route-preference"); if (rp && (v = lyd_get_value(rp))) { route.route_preference = static_cast<uint32_t>(std::stoul(v)); }
					cp.static_routes.push_back(route);
				}
			}

			model->mutableRouting().control_plane_protocols.push_back(cp);
		}
	}

	// ribs
	struct lyd_node *ribs = find_child(rt, "ribs");
	if (ribs) {
		for (struct lyd_node *entry = lyd_child(ribs); entry; entry = entry->next) {
			if (!entry->schema || !entry->schema->name) continue;
			if (strcmp(entry->schema->name, "rib") != 0) continue;
			IetfRouting::Rib rib;
			struct lyd_node *nnode = find_child(entry, "name"); if (nnode && (v = lyd_get_value(nnode))) rib.name = v;
			struct lyd_node *af = find_child(entry, "address-family"); if (af && (v = lyd_get_value(af))) rib.address_family = v;
			struct lyd_node *desc = find_child(entry, "description"); if (desc && (v = lyd_get_value(desc))) rib.description = std::string(v);

			struct lyd_node *routes = find_child(entry, "routes");
			if (routes) {
				for (struct lyd_node *r = lyd_child(routes); r; r = r->next) {
					if (!r->schema || !r->schema->name) continue;
					if (strcmp(r->schema->name, "route") != 0) continue;
					IetfRouting::Route route;
					struct lyd_node *rp = find_child(r, "route-preference"); if (rp && (v = lyd_get_value(rp))) { route.route_preference = static_cast<uint32_t>(std::stoul(v)); }
					rib.routes.push_back(route);
				}
			}

			model->mutableRouting().ribs.push_back(rib);
		}
	}

	return model;
}

