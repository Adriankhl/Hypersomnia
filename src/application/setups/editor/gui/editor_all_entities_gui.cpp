#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"

#include "application/setups/editor/property_editor/flavours_and_entities_tree.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

using resolved_array_type = std::array<
	std::unordered_map<raw_entity_flavour_id, std::vector<entity_id>>,
	num_types_in_list_v<all_entity_types>
>;

template <class C>
void sort_flavours_by_name(const cosmos& cosm, C& ids) {
	sort_range_by(
		ids,
		[&](const auto id) -> const std::string* { 
			return std::addressof(cosm.get_flavour(id).template get<invariants::name>().name);
		},
		[](const auto& a, const auto& b) {
			return *a.compared < *b.compared;
		}
	);
}

class in_selection_provider {
	const cosmos& cosm;
	const resolved_array_type& per_native_type;

	template <class E>
	const auto& get_map() const {
		return per_native_type[entity_type_id::of<E>().get_index()];
	}

public:
	in_selection_provider(
		const cosmos& cosm,
		const resolved_array_type& per_native_type
	) : 
		cosm(cosm),
		per_native_type(per_native_type)
	{}

	template <class E>
	auto num_flavours_of_type() const {
		return get_map<E>().size();
	}

	template <class E>
	auto num_entities_of_type() const {
		std::size_t total = 0;

		for (const auto& p : get_map<E>()) {
			total += p.second.size();
		}

		return total;
	}

	template <class E>
	const auto& get_all_flavour_ids() const {
		thread_local std::vector<entity_flavour_id> ids;
		ids.clear();

		for (const auto& p : get_map<E>()) {
			entity_flavour_id flavour_id;
			flavour_id.type_id = entity_type_id::of<E>();
			flavour_id.raw = p.first;
			ids.push_back(flavour_id);
		}

		return ids;
	}

	auto get_entities_by_flavour_id(const entity_flavour_id id) const {
		auto ids = per_native_type[id.type_id.get_index()].at(id.raw);

		sort_range_by(
			ids,
			[&](const auto id) { 
				return cosm[id].get_guid();
			}
		);

		return ids;
	}

	template <class E, class F>
	void for_each_flavour(F callback) const {
		thread_local std::vector<typed_entity_flavour_id<E>> ids;
		ids.clear();

		const auto& all_flavours = get_map<E>();

		for (const auto& f : all_flavours) {
			const auto id = typed_entity_flavour_id<E>(f.first);
			ids.push_back(id);
		}

		sort_flavours_by_name(cosm, ids);

		for (const auto& id : ids) {
			const auto& flavour = cosm.get_flavour(id);
			callback(id, flavour);
		}
	}
};

class all_provider {
	const cosmos& cosm;

	const auto& common() const {
		return cosm.get_common_significant();
	}

public:
	all_provider(const cosmos& cosm) : cosm(cosm) {}

	template <class E>
	auto num_flavours_of_type() const {
		return common().get_flavours<E>().count();
	}

	template <class E>
	auto num_entities_of_type() const {
		return cosm.get_solvable().get_count_of<E>();
	}

	template <class E>
	const auto& get_all_flavour_ids() const {
		thread_local std::vector<entity_flavour_id> all_flavour_ids;
		all_flavour_ids.clear();

		const auto& all_flavours = common().get_flavours<E>();

		all_flavours.for_each([&](
			const auto flavour_id,
			const auto& flavour
		) {
			all_flavour_ids.push_back(flavour_id);
		});

		return all_flavour_ids;
	}

	const auto& get_entities_by_flavour_id(const entity_flavour_id id) const {
		return cosm.get_solvable_inferred().name.get_entities_by_flavour_id(id);
	}

	template <class E, class F>
	void for_each_flavour(F callback) const {
		thread_local std::vector<typed_entity_flavour_id<E>> ids;
		ids.clear();
		ids.reserve(num_flavours_of_type<E>());

		const auto& all_flavours = common().get_flavours<E>();

		all_flavours.for_each([](const auto& id, const auto&){
			ids.push_back(id);	
		});

		sort_flavours_by_name(cosm, ids);

		for (const auto& id : ids) {
			callback(id, cosm.get_flavour(id));
		}
	}
};

void editor_all_entities_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus(title.c_str());
}

void editor_all_entities_gui::interrupt_tweakers() {
	properties_gui.last_active.reset();
	properties_gui.old_description.clear();
}


void editor_all_entities_gui::perform(
	const std::unordered_set<entity_id>* only_match_entities,
	editor_command_input in
) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	auto entities = scoped_window(title.c_str(), &show);

	ImGui::Columns(2);
	next_column_text_disabled("Details");
	ImGui::Separator();

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	properties_gui.hovered_guid.unset();

	auto& cosm = in.folder.work->world;

	if (only_match_entities) {
		const auto& matches = *only_match_entities;
		const auto num_matches = matches.size();

		if (num_matches == 0) {
			return;
		}
		if (num_matches == 1) {
			const auto id = *matches.begin();

			if (const auto handle = cosm[id]) {
				handle.dispatch([&](const auto typed_handle) {
					do_edit_flavours_gui(properties_gui, in, typed_handle.get_flavour(), typed_handle.get_flavour_id());
					do_edit_entities_gui(properties_gui, in, typed_handle, id);
				});
			}
		}
		else if (num_matches > 1) {
			thread_local resolved_array_type per_native_type;

			for (auto& p : per_native_type) {
				p.clear();
			}

			for (const auto& e : matches) {
				if (const auto handle = cosm[e]) {
					per_native_type[e.type_id.get_index()][handle.get_flavour_id().raw].push_back(e);
				}
			}

			flavours_and_entities_tree(
				properties_gui,
				in,
				in_selection_provider { cosm, per_native_type }
			);
		}

		return;
	}

	flavours_and_entities_tree(
		properties_gui,
		in,
		all_provider { cosm }
	);
}
