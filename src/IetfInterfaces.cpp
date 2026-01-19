#include "IetfInterfaces.hpp"

#include <libyang/libyang.h>

using namespace yang;

#include "IetfInterfaces.hpp"

#include "Exceptions.hpp"
#include <libyang/libyang.h>

using namespace yang;

struct lyd_node* IetfInterfaces::serialize(const YangContext &ctx) const {
	struct ly_ctx *c = ctx.raw();

	struct lyd_node *root = nullptr;
	LY_ERR err = lyd_new_path(nullptr, c, "/ietf-interfaces:interfaces", NULL, 0, &root);
	if (err != LY_SUCCESS || !root) throw YangDataError(ctx);

	for (const auto &it : ifs_) {
		std::string name = it.name;
		std::string rel_name = "interface[name='" + name + "']/name";
		{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_name.c_str(), name.c_str(), 0, &tmp); }
		if (it.description.has_value()) {
			std::string rel_desc = "interface[name='" + name + "']/description";
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_desc.c_str(), it.description->c_str(), 0, &tmp); }
		}
		if (!it.enabled) {
			std::string rel_en = "interface[name='" + name + "']/enabled";
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_en.c_str(), "false", 0, &tmp); }
		}
		if (it.type.has_value()) {
			std::string rel_type = "interface[name='" + name + "']/type";
			std::string t = "iana-if-type:" + yang::toString(*it.type);
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_type.c_str(), t.c_str(), 0, &tmp); }
		}
		if (it.mtu.has_value()) {
			std::string rel_mtu = "interface[name='" + name + "']/mtu";
			{ struct lyd_node *tmp = nullptr; lyd_new_path(root, c, rel_mtu.c_str(), std::to_string(*it.mtu).c_str(), 0, &tmp); }
		}
	}
	return root;
}

std::unique_ptr<IetfInterfaces> IetfInterfaces::deserialize(const YangContext &ctx, struct lyd_node* tree) {
	if (!tree) throw YangDataError(ctx);

	auto model = std::make_unique<IetfInterfaces>();

	struct lyd_node *ifs = nullptr;
	if (tree->schema && tree->schema->name && strcmp(tree->schema->name, "interfaces") == 0) {
		ifs = tree;
	} else {
		if (lyd_find_path(tree, "/ietf-interfaces:interfaces", 0, &ifs) != LY_SUCCESS) ifs = nullptr;
	}
	if (!ifs) throw YangDataError(ctx);

	auto find_child = [](struct lyd_node *p, const char *n) -> struct lyd_node* {
		if (!p) return nullptr;
		for (struct lyd_node *c = lyd_child(p); c; c = c->next) {
			if (c->schema && c->schema->name && strcmp(c->schema->name, n) == 0) return c;
		}
		return nullptr;
	};

	const char *v = nullptr;
	for (struct lyd_node *ch = lyd_child(ifs); ch; ch = ch->next) {
		if (!ch->schema || !ch->schema->name) continue;
		if (strcmp(ch->schema->name, "interface") != 0) continue;
		IetfInterfaces::IetfInterface itf;
		struct lyd_node *n = find_child(ch, "name");
		if (n && (v = lyd_get_value(n))) itf.name = v;
		if (itf.name.empty()) continue; // skip malformed
		n = find_child(ch, "description"); if (n && (v = lyd_get_value(n))) itf.description = std::string(v);
		n = find_child(ch, "type"); if (n && (v = lyd_get_value(n))) {
			std::string sv(v);
			if (sv.find("ethernetCsmacd") != std::string::npos) itf.type = yang::IanaIfType::ethernetCsmacd;
			else itf.type = yang::IanaIfType::Unknown;
		}
		n = find_child(ch, "mtu"); if (n && (v = lyd_get_value(n))) itf.mtu = static_cast<uint32_t>(std::stoul(v));
		n = find_child(ch, "enabled"); if (n && (v = lyd_get_value(n))) itf.enabled = !(strcmp(v, "false") == 0 || strcmp(v, "0") == 0);
		n = find_child(ch, "last-change"); if (n && (v = lyd_get_value(n))) itf.last_change = std::string(v);
		n = find_child(ch, "if-index"); if (n && (v = lyd_get_value(n))) itf.if_index = std::stoi(v);
		n = find_child(ch, "phys-address"); if (n && (v = lyd_get_value(n))) itf.phys_address = std::string(v);

		// higher-layer-if and lower-layer-if are leaf-lists
		for (struct lyd_node *ll = lyd_child(ch); ll; ll = ll->next) {
			if (!ll->schema || !ll->schema->name) continue;
			if (strcmp(ll->schema->name, "higher-layer-if") == 0) { if ((v = lyd_get_value(ll))) itf.higher_layer_if.push_back(v); }
			if (strcmp(ll->schema->name, "lower-layer-if") == 0) { if ((v = lyd_get_value(ll))) itf.lower_layer_if.push_back(v); }
		}

		model->addInterface(itf);
	}

	return model;
}

