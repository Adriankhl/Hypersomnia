#pragma once
#include <unordered_map>

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/cosmic_entropy.h"

#include "view/game_gui/game_gui_context.h"
#include "view/game_gui/elements/game_gui_root.h"
#include "view/game_gui/game_gui_intent_type.h"

struct character_gui;

class game_gui_system {
public:
	std::unordered_map<entity_id, item_button> item_buttons;
	std::unordered_map<inventory_slot_id, slot_button> slot_buttons;

	std::unordered_map<entity_id, character_gui> character_guis;

	augs::container_with_small_size<std::vector<item_slot_transfer_request>, unsigned short> pending_transfers;
	augs::container_with_small_size<std::unordered_map<entity_id, spell_id>, unsigned char> spell_requests;
	
	game_gui_rect_world world;
	game_gui_rect_tree tree;
	game_gui_root root;

	bool active = false;

	game_gui_context create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const const_entity_handle gui_entity,
		const game_gui_context_dependencies deps
	) {
		return {
			{ world, tree, screen_size, input_state },
			*this,
			gui_entity,
			get_character_gui(gui_entity),
			deps
		};
	}

	const_game_gui_context create_context(
		const vec2i screen_size,
		const augs::event::state input_state,
		const const_entity_handle gui_entity,
		const game_gui_context_dependencies deps
	) const {
		return {
			{ world, tree, screen_size, input_state },
			*this,
			gui_entity,
			get_character_gui(gui_entity),
			deps
		};
	}

	cosmic_entropy get_and_clear_pending_events();
	void clear_all_pending_events();

	void queue_transfer(const item_slot_transfer_request);
	void queue_transfers(const wielding_result);

	character_gui& get_character_gui(const entity_id);
	const character_gui& get_character_gui(const entity_id) const;

	slot_button& get_slot_button(const inventory_slot_id);
	const slot_button& get_slot_button(const inventory_slot_id) const;

	item_button& get_item_button(const entity_id);
	const item_button& get_item_button(const entity_id) const;

	void control_gui_world(
		const game_gui_context context,
		const augs::event::change change
	);

	void control_hotbar_and_action_button(
		const const_entity_handle root_entity,
		const game_gui_intent intent
	);

	void advance(
		const game_gui_context context,
		const augs::delta dt
	);

	void build_tree_data(const game_gui_context);

	void rebuild_layouts(
		const game_gui_context context
	);

	void reserve_caches_for_entities(const size_t) const {}

	void standard_post_solve(const const_logic_step);
	void standard_post_cleanup(const const_logic_step);

	void erase_caches_for_dead_entities(const cosmos&);
};