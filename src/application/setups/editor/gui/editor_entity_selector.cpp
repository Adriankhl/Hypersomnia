#include "application/setups/editor/gui/editor_entity_selector.h"

#include "game/transcendental/cosmos.h"

void editor_entity_selector::clear() {
	in_rectangular_selection.clear();
	hovered.unset();
};

std::optional<ltrb> editor_entity_selector::get_screen_space_rect_selection(
	const camera_cone& camera,
	vec2i screen_size,
	vec2i mouse_pos
) const {
	if (rectangular_drag_origin.has_value()) {
		return ltrb::from_points(
			mouse_pos,
			camera.to_screen_space(screen_size, *rectangular_drag_origin)
		);
	}

	return std::nullopt;
}

void editor_entity_selector::do_left_press(
	const cosmos& cosm,
	bool has_ctrl,
	const vec2i world_cursor_pos,
	target_selections_type& selections
) {
	last_ldown_position = world_cursor_pos;
	held = hovered;

	if (const auto held_entity = cosm[held]) {
		flavour_of_held = held_entity.get_flavour_id();
		LOG_NVPS(flavour_of_held);
	}

	if (!has_ctrl) {
		/* Make a new selection */
		selections.clear();
	}
}

void editor_entity_selector::finish_rectangular(target_selections_type& into) {
	target_selections_type new_selections;

	for_each_selected_entity(
		[&](const auto e) {
			new_selections.emplace(e);
		},
		into
	);

	into = new_selections;

	rectangular_drag_origin = std::nullopt;
	in_rectangular_selection.clear();
	flavour_of_held = {};
}

void editor_entity_selector::do_left_release(
	const bool has_ctrl,
	target_selections_type& selections
) {
	if (const auto clicked = held) {
		if (has_ctrl) {
			if (found_in(selections, clicked)) {
				selections.erase(clicked);
			}
			else {
				selections.emplace(clicked);
			}
		}
		else {
			selections = { clicked };
		}
	}

	held = {};

	finish_rectangular(selections);
}

void editor_entity_selector::unhover() {
	hovered = {};
}

static bool should_hover_standard_aabb(const cosmos& cosm, const entity_id id) {
	return cosm[id].dispatch([](const auto typed_handle){
		using T = std::decay_t<decltype(typed_handle)>;
		using E = typename T::used_entity_type;

		if (std::is_same_v<E, wandering_pixels_decoration>) {
			return false;
		}
		else if (std::is_same_v<E, static_light>) {
			return false;
		}

		return true;
	});
};

void editor_entity_selector::do_mousemotion(
	const necessary_images_in_atlas& sizes_for_icons,

	const cosmos& cosm,
	const editor_rect_select_type rect_select_mode,
	const vec2 world_cursor_pos,
	const camera_cone current_cone,
	const bool left_button_pressed
) {
	hovered = {};

	auto get_world_xywh = [&](const auto icon_id, const components::transform where) {
		return xywh::center_and_size(where.pos, vec2(sizes_for_icons.at(icon_id).get_size()) / current_cone.zoom).expand_to_square();
	};

	{
		const bool drag_just_left_dead_area = [&]() {
			const auto drag_dead_area = 3.f;
			const auto drag_offset = world_cursor_pos - last_ldown_position;

			return !drag_offset.is_epsilon(drag_dead_area);
		}();

		if (left_button_pressed && drag_just_left_dead_area) {
			rectangular_drag_origin = last_ldown_position;
			held = {};
		}
	}

	if (rectangular_drag_origin.has_value()) {
		auto world_range = ltrb::from_points(*rectangular_drag_origin, world_cursor_pos);

		in_rectangular_selection.clear();

		const auto query = visible_entities_query{
			cosm,
			{ world_range.get_center(), 1.f },
			world_range.get_size()
		};

		in_rectangular_selection.acquire_non_physical(query);
		in_rectangular_selection.acquire_physical(query);

		erase_if(in_rectangular_selection.all, [&](const entity_id id) {
			return !should_hover_standard_aabb(cosm, id);
	   	});

		for_each_iconed_entity(cosm, 
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto color
			) {
				if (get_world_xywh(tex_id, where).hover(world_range)) {
					in_rectangular_selection.all.push_back(handle.get_id());
				}
			}
		);

		if (rect_select_mode == editor_rect_select_type::SAME_FLAVOUR) {
			erase_if(in_rectangular_selection.all, [&](const entity_id id) {
				return !(cosm[id].get_flavour_id() == flavour_of_held);
			});
		}
	}
	else {
		hovered.unset();

		for_each_iconed_entity(cosm, 
			[&](const auto handle, 
				const auto tex_id,
			   	const auto where,
				const auto color
			) {
				if (get_world_xywh(tex_id, where).hover(world_cursor_pos)) {
					hovered = handle.get_id();
				}
			}
		);

		if (!hovered) {
			hovered = get_hovered_world_entity(
				cosm, 
				world_cursor_pos, 
				[&](const entity_id id) { 
					return should_hover_standard_aabb(cosm, id);
				}
			);
		}
	}
}

std::optional<ltrb> editor_entity_selector::get_selection_aabb(
	const cosmos& cosm,
	const target_selections_type& signi_selections
) const {
	ltrb total;

	auto combine_aabb_of = [&total, &cosm](const entity_id id) {
		const auto handle = cosm[id];

		if (const auto aabb = handle.find_aabb()) {
			total.contain(*aabb);	
		}
	};

	if (held && cosm[held]) {
		combine_aabb_of(held);
	}

	for_each_selected_entity(
		combine_aabb_of,
		signi_selections
	);

	if (total.good()) {
		return total;
	}

	return std::nullopt;
}

std::optional<rgba> editor_entity_selector::get_highlight_color_of(
	const editor_entity_selector_settings& settings,
	const entity_id id, 
	const target_selections_type& signi_selections
) const {
	if (held == id) {
		return settings.held_color;
	}

	if (hovered == id) {
		return settings.hovered_color;
	}

	const bool in_signi = found_in(signi_selections, id);
	const bool in_rectangular = found_in(in_rectangular_selection.all, id);

	if (in_signi != in_rectangular) {
		return settings.selected_color;
	}

	return std::nullopt;
}
