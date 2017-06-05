#include "gui_element_system.h"
#include "game/detail/gui/game_gui_element_location.h"
#include "game/detail/gui/character_gui.h"
#include "augs/graphics/renderer.h"

#include "game/components/item_component.h"

#include "game/systems_stateless/crosshair_system.h"
#include "game/transcendental/entity_handle.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/inventory/inventory_slot_handle.h"
#include "game/components/container_component.h"
#include "game/components/item_slot_transfers_component.h"
#include "game/components/item_component.h"

#include "augs/gui/rect_world.h"
#include "game/enums/slot_function.h"
#include "game/detail/gui/root_of_inventory_gui.h"

#include "game/detail/inventory/inventory_utils.h"

#include "augs/gui/button_corners.h"

#include "game/detail/gui/slot_button.h"
#include "game/detail/gui/item_button.h"

static char intent_to_hotbar_index(const intent_type type) {
	switch (type) {
	case intent_type::HOTBAR_BUTTON_0: return 0;
	case intent_type::HOTBAR_BUTTON_1: return 1;
	case intent_type::HOTBAR_BUTTON_2: return 2;
	case intent_type::HOTBAR_BUTTON_3: return 3;
	case intent_type::HOTBAR_BUTTON_4: return 4;
	case intent_type::HOTBAR_BUTTON_5: return 5;
	case intent_type::HOTBAR_BUTTON_6: return 6;
	case intent_type::HOTBAR_BUTTON_7: return 7;
	case intent_type::HOTBAR_BUTTON_8: return 8;
	case intent_type::HOTBAR_BUTTON_9: return 9;
	default: return -1;
	}
}

static char intent_to_special_action_index(const intent_type type) {
	switch (type) {
	case intent_type::SPECIAL_ACTION_BUTTON_1: return 0;
	case intent_type::SPECIAL_ACTION_BUTTON_2: return 1;
	case intent_type::SPECIAL_ACTION_BUTTON_3: return 2;
	case intent_type::SPECIAL_ACTION_BUTTON_4: return 3;
	case intent_type::SPECIAL_ACTION_BUTTON_5: return 4;
	case intent_type::SPECIAL_ACTION_BUTTON_6: return 5;
	case intent_type::SPECIAL_ACTION_BUTTON_7: return 6;
	case intent_type::SPECIAL_ACTION_BUTTON_8: return 7;
	case intent_type::SPECIAL_ACTION_BUTTON_9: return 8;
	case intent_type::SPECIAL_ACTION_BUTTON_10: return 9;
	case intent_type::SPECIAL_ACTION_BUTTON_11: return 10;
	case intent_type::SPECIAL_ACTION_BUTTON_12: return 11;
	default: return -1;
	}
}

cosmic_entropy gui_element_system::get_and_clear_pending_events() {
	cosmic_entropy out;

	out.transfer_requests = pending_transfers;
	out.cast_spells_per_entity = spell_requests;

	pending_transfers.clear();
	spell_requests.clear();

	return out;
}

void gui_element_system::clear_all_pending_events() {
	pending_transfers.clear();
	spell_requests.clear();
}

character_gui& gui_element_system::get_character_gui(const entity_id id) {
	const auto it = character_guis.find(id);

	if (it == character_guis.end()) {
		auto& new_gui = character_guis[id];
		new_gui.set_screen_size(screen_size_for_new_characters);
		
		new_gui.action_buttons[0].bound_spell.set<haste_instance>();
		new_gui.action_buttons[1].bound_spell.set<fury_of_the_aeons_instance>();
		new_gui.action_buttons[2].bound_spell.set<ultimate_wrath_of_the_aeons_instance>();
		new_gui.action_buttons[3].bound_spell.set<electric_triad_instance>();
		new_gui.action_buttons[4].bound_spell.set<electric_shield_instance>();
		
		return new_gui;
	}
	
	return (*it).second;
}

const character_gui& gui_element_system::get_character_gui(const entity_id id) const {
	return character_guis.at(id);
}

slot_button& gui_element_system::get_slot_button(const inventory_slot_id id) {
	return slot_buttons[id];
}

const slot_button& gui_element_system::get_slot_button(const inventory_slot_id id) const {
	return slot_buttons.at(id);
}

item_button& gui_element_system::get_item_button(const entity_id id) {
	return item_buttons[id];
}

const item_button& gui_element_system::get_item_button(const entity_id id) const {
	return item_buttons.at(id);
}

void gui_element_system::queue_transfer(const item_slot_transfer_request req) {
	pending_transfers.push_back(req);
}

void gui_element_system::queue_transfers(const wielding_result res) {
	// ensure(res.successful());

	if (res.successful()) {
		concatenate(pending_transfers, res.transfers);
	}
}

void gui_element_system::handle_hotbar_and_action_button_presses(
	const const_entity_handle gui_entity,
	const game_intent_vector& intents
) {
	const auto& cosmos = gui_entity.get_cosmos();
	auto& gui = get_character_gui(gui_entity);

	for (auto r : intents) {
		if (r.is_pressed) {
			int hand_index = -1;

			if (r.intent == intent_type::HOLSTER) {
				if (gui_entity.get_if_any_item_in_hand_no(0).alive()) {
					r.intent = intent_type::HOLSTER_PRIMARY_ITEM;
				}
				else if (
					gui_entity.get_if_any_item_in_hand_no(0).dead()
					&& gui_entity.get_if_any_item_in_hand_no(1).alive()
				) {
					r.intent = intent_type::HOLSTER_SECONDARY_ITEM;
				}
			}

			if (r.intent == intent_type::HOLSTER_PRIMARY_ITEM) {
				hand_index = 0;
			}
			else if (r.intent == intent_type::HOLSTER_SECONDARY_ITEM) {
				hand_index = 1;
			}

			if (hand_index >= 0) {
				auto new_setup = gui.get_actual_selection_setup(gui_entity);
				new_setup.hand_selections[static_cast<size_t>(hand_index)].unset();
				queue_transfers(gui.make_and_push_hotbar_selection_setup(new_setup, gui_entity));
			}
		}
	}

	for (const auto& i : intents) {
		const auto hotbar_button_index = intent_to_hotbar_index(i.intent);
		const auto special_action_index = intent_to_special_action_index(i.intent);

		if (hotbar_button_index >= 0 && static_cast<std::size_t>(hotbar_button_index) < gui.hotbar_buttons.size()) {
			auto& currently_held_index = gui.currently_held_hotbar_button_index;

			const bool clear_currently_held_index_because_nothing_is_assigned_already =
				currently_held_index > -1 && gui.hotbar_buttons[currently_held_index].get_assigned_entity(gui_entity).dead()
			;

			if (clear_currently_held_index_because_nothing_is_assigned_already) {
				currently_held_index = -1;
			}

			const bool is_anything_assigned_to_that_button = gui.hotbar_buttons[hotbar_button_index].get_assigned_entity(gui_entity).alive();

			if (is_anything_assigned_to_that_button) {
				if (i.is_pressed) {
					const bool should_dual_wield = currently_held_index > -1;

					if (should_dual_wield) {
						const auto setup = gui.get_setup_from_button_indices(
							gui_entity, 
							currently_held_index, 
							hotbar_button_index
						);

						queue_transfers(gui.make_and_push_hotbar_selection_setup(setup, gui_entity));
						gui.push_new_setup_when_index_released = -1;
					}
					else {
						const auto setup = gui.get_setup_from_button_indices(
							gui_entity, 
							hotbar_button_index
						);

						queue_transfers(gui.make_wielding_transfers_for(setup, gui_entity));
						gui.push_new_setup_when_index_released = hotbar_button_index;
					}

					currently_held_index = hotbar_button_index;
				}
				else {
					if (hotbar_button_index == currently_held_index) {
						currently_held_index = -1;
					}
				
					if (hotbar_button_index == gui.push_new_setup_when_index_released) {
						const auto setup = gui.get_setup_from_button_indices(gui_entity, hotbar_button_index);
						gui.push_setup(setup);
				
						gui.push_new_setup_when_index_released = -1;
					}
				}
			}
		}
		else if (special_action_index >= 0 && static_cast<std::size_t>(special_action_index) < gui.action_buttons.size()) {
			auto& action_b = gui.action_buttons[special_action_index];
			action_b.detector.update_appearance(i.is_pressed ? gui_event::ldown : gui_event::lup);

			if (i.is_pressed) {
				const auto bound_spell = action_b.bound_spell;

				if (bound_spell.is_set() && gui_entity.get<components::sentience>().is_learned(bound_spell)) {
					spell_requests[gui_entity] = bound_spell;
				}
			}
		}
		else if (i.intent == intent_type::PREVIOUS_HOTBAR_SELECTION_SETUP && i.is_pressed) {
			const auto wielding = gui.make_wielding_transfers_for_previous_hotbar_selection_setup(gui_entity);

			if (wielding.successful()) {
				queue_transfers(wielding);
			}
		}
	}
}

void gui_element_system::advance_elements(
	const const_entity_handle root_entity,
	const augs::delta dt
) {
	const auto& cosmos = root_entity.get_cosmos();

	auto& element = get_character_gui(root_entity);
	auto& rect_world = element.rect_world;
	const auto screen_size = element.get_screen_size();

	ensure(root_entity.has<components::item_slot_transfers>());

	game_gui_rect_tree tree;

	root_of_inventory_gui root_of_gui(screen_size);

	game_gui_context context(
		*this,
		element.rect_world,
		element,
		root_entity,
		tree,
		root_of_gui
	);

	root_of_inventory_gui_in_context root_location;

	rect_world.advance_elements(
		context,
		root_location,
		dt
	);
}
	
void gui_element_system::control_gui(
	const const_entity_handle root_entity,
	std::vector<augs::window::event::change>& events
) {
	const auto& cosmos = root_entity.get_cosmos();

	auto& element = get_character_gui(root_entity);
	auto& rect_world = element.rect_world;
	const auto screen_size = element.get_screen_size();

	ensure(root_entity.has<components::item_slot_transfers>());
	
	game_gui_rect_tree tree;
	augs::gui::gui_entropy<game_gui_element_location> gui_events;

	root_of_inventory_gui root_of_gui(screen_size);

	game_gui_context context(
		*this,
		element.rect_world, 
		element, 
		root_entity, 
		tree, 
		root_of_gui
	);

	root_of_inventory_gui_in_context root_location;

	rect_world.build_tree_data_into_context(context, root_location);

	if (gui_look_enabled) {
		erase_if(events, [&](const augs::window::event::change change) {
			bool fetched = false;

			const auto held_rect = context.get_if<item_button_in_item>(rect_world.rect_held_by_lmb);

			if (held_rect != nullptr) {
				const auto& item_entity = cosmos[held_rect.get_location().item_id];
				auto& dragged_charges = element.dragged_charges;

				if (
					change.msg == augs::window::event::message::rdown
					|| change.msg == augs::window::event::message::rdoubleclick
				) {
					if (rect_world.held_rect_is_dragged) {
						pending_transfers.push_back(item_slot_transfer_request { item_entity, cosmos[inventory_slot_id()], dragged_charges });
						fetched = true;
					}
				}

				if (change.msg == augs::window::event::message::wheel) {
					const auto& item = item_entity.get<components::item>();

					const auto delta = change.scroll.amount;

					dragged_charges += delta;

					if (dragged_charges <= 0) {
						dragged_charges = item.charges + dragged_charges;
					}
					if (dragged_charges > item.charges) {
						dragged_charges = dragged_charges - item.charges;
					}
				}
			}

			if (!fetched) {
				rect_world.consume_raw_input_and_generate_gui_events(
					context,
					root_location,
					change,
					gui_events
				);
			}

			return change.uses_mouse();
		});
	}

	// rect_world.call_idle_mousemotion_updater(context, root_location, entropy);
	rect_world.respond_to_events(
		context, 
		root_location, 
		gui_events
	);
}

void gui_element_system::rebuild_layouts(
	const const_entity_handle root_entity
) {
	const auto& cosmos = root_entity.get_cosmos();

	auto& element = get_character_gui(root_entity);
	auto& rect_world = element.rect_world;
	const auto screen_size = element.get_screen_size();

	ensure(root_entity.has<components::item_slot_transfers>());

	game_gui_rect_tree tree;

	root_of_inventory_gui root_of_gui(screen_size);

	game_gui_context context(
		*this,
		element.rect_world,
		element,
		root_entity,
		tree,
		root_of_gui
	);

	int max_hotbar_height = 0;

	{
		int total_width = 0;

		for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
			const auto& hb = element.hotbar_buttons[i];

			const auto bbox = hb.get_bbox(root_entity);
			max_hotbar_height = std::max(max_hotbar_height, bbox.y);

			total_width += bbox.x;
		}

		const int left_rc_spacing = 2;
		const int right_rc_spacing = 1;

		int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

		const auto set_rc = [&](auto& hb) {
			const auto bbox = hb.get_bbox(root_entity);

			hb.rc = xywh(xywhi(current_x, screen_size.y - max_hotbar_height - 50, bbox.x + left_rc_spacing + right_rc_spacing, max_hotbar_height));

			current_x += bbox.x + left_rc_spacing + right_rc_spacing;
		};

		for (size_t i = 0; i < element.hotbar_buttons.size(); ++i) {
			set_rc(element.hotbar_buttons[i]);
		}
	}

	{
		const auto action_button_size = get_assets_manager()[assets::game_image_id::ACTION_BUTTON_BORDER].texture_maps[texture_map_type::DIFFUSE].get_size();

		int total_width = element.action_buttons.size() * action_button_size.x;

		const int left_rc_spacing = 4;
		const int right_rc_spacing = 3;

		int current_x = screen_size.x / 2 - total_width / 2 - left_rc_spacing;

		const auto set_rc = [&](auto& hb) {
			const auto bbox = action_button_size;

			hb.rc = xywh(xywhi(
				current_x, screen_size.y - action_button_size.y - 9 - max_hotbar_height - 50, 
				bbox.x + left_rc_spacing + right_rc_spacing, 
				action_button_size.y));

			current_x += bbox.x + left_rc_spacing + right_rc_spacing;
		};

		for (size_t i = 0; i < element.action_buttons.size(); ++i) {
			set_rc(element.action_buttons[i]);
		}
	}

	root_of_inventory_gui_in_context root_location;
	rect_world.rebuild_layouts(context, root_location);
}

void gui_element_system::reposition_picked_up_and_transferred_items(const const_logic_step step) {

}

void gui_element_system::erase_caches_for_dead_entities(const cosmos& new_cosmos) {
	const auto eraser = [&](auto& caches) {
		erase_if(caches, [&](const auto& it) {
			return new_cosmos[it.first].dead();
		});
	};

	eraser(character_guis);
	eraser(item_buttons);
	eraser(slot_buttons);
}