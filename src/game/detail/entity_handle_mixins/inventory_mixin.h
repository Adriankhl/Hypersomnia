#pragma once
#include <optional>

#include "augs/templates/container_templates.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_id.h"
#include "augs/build_settings/platform_defines.h"
#include "game/detail/inventory/wielding_result.h"
#include "augs/callback_result.h"

template <class derived_handle_type>
class inventory_mixin {
	static void sum(colliders_connection& into, const colliders_connection from) {
		into.owner = from.owner;
		// TODO: Sum from a matrix
		into.shape_offset += components::transform();
	}

public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;

	using generic_handle_type = basic_entity_handle<is_const>;
	using inventory_slot_handle_type = basic_inventory_slot_handle<generic_handle_type>;

	static constexpr size_t hand_count = 2;
	typedef std::array<entity_id, hand_count> hand_selections_array;

	std::optional<unsigned> get_space_occupied() const;	

	generic_handle_type get_owning_transfer_capability() const;
	bool owning_transfer_capability_alive_and_same_as_of(const entity_id) const;

	std::optional<colliders_connection> calc_connection_to_topmost_container() const {
		const auto& self = *static_cast<const derived_handle_type*>(this);
		ensure(self);

		const auto& cosmos = self.get_cosmos();

		colliders_connection result;

		inventory_slot_id it = self.get_current_slot();

		while (const auto slot = cosmos[it]) {
			if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
				/* 
					Failed: "until" not found before meeting an item deposit.
					This nullopt will be used to determine
					that the fixtures for this item should be deactivated now.
				*/

				return std::nullopt;
			}

			ensure(slot->physical_behaviour == slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY); 

			sum(result, { it.container_entity });

			it = slot.get_container().get_current_slot();
		}
		
		return result;
	}

	std::optional<colliders_connection> calc_connection_until_container(const entity_id until) const {
		const auto& self = *static_cast<const derived_handle_type*>(this);

		ensure(self);
		ensure(until.is_set());

		const auto& cosmos = self.get_cosmos();

		if (until == self) {
			return colliders_connection { self, {} };
		}

		colliders_connection result;

		inventory_slot_id it = self.get_current_slot();

		do {
			const auto slot = cosmos[it];

			if (slot.dead()) {
				/* Failed: found a dead slot before could reach "until" */
				return std::nullopt;
			}

			if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
				/* Failed: "until" not found before meeting an item deposit. */
				return std::nullopt;
			}

			ensure(slot->physical_behaviour == slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY); 

			sum(result, { it.container_entity });

			it = slot.get_container().get_current_slot();
		} while(it.container_entity != until);

		return result;
	}

	template <class handle_type>
	inventory_slot_handle_type determine_holstering_slot_for(const handle_type holstered_item) const {
		const auto& searched_root_container = *static_cast<const derived_handle_type*>(this);
		auto& cosmos = holstered_item.get_cosmos();

		ensure(holstered_item.alive()) 
		ensure(searched_root_container.alive());
		ensure(holstered_item.get_id() != searched_root_container.get_id());

		inventory_slot_id target_slot;

		searched_root_container.for_each_contained_slot_recursive(
			[&](const auto slot) {
				if (slot.get_container() == holstered_item) {
					return recursive_callback_result::CONTINUE_DONT_RECURSE;
				}

				if (!slot.is_hand_slot() && slot.can_contain(holstered_item)) {
					target_slot = slot;
					return recursive_callback_result::ABORT;
				}

				return recursive_callback_result::CONTINUE_AND_RECURSE;
			}
		);

		return cosmos[target_slot];
	}

	template <class handle_type>
	inventory_slot_handle_type determine_pickup_target_slot_for(const handle_type picked_item) const {
		const auto& searched_root_container = *static_cast<const derived_handle_type*>(this);

		ensure(picked_item.alive());
		ensure(searched_root_container.alive());

		auto& cosmos = picked_item.get_cosmos();

		inventory_slot_id target_slot;

		const auto holster_slot = searched_root_container.determine_holstering_slot_for(picked_item);

		if (holster_slot.alive()) {
			target_slot = holster_slot;
		}
		else {
			searched_root_container.for_each_hand(
				[&](const auto hand) {
					if (hand.can_contain(picked_item)) {
						target_slot = hand;

						return callback_result::ABORT;
					}

					return callback_result::CONTINUE;
				}
			);
		}

		return cosmos[target_slot];
	}
	
	inventory_slot_handle_type get_current_slot() const;

	inventory_slot_handle_type get_first_free_hand() const;
	
	inventory_slot_handle_type get_primary_hand() const;
	inventory_slot_handle_type get_secondary_hand() const;

	inventory_slot_handle_type get_hand_no(const size_t) const;
	generic_handle_type get_if_any_item_in_hand_no(const size_t) const;

	template <class F>
	void for_each_hand(F callback) const {
		for (size_t i = 0; i < hand_count; ++i) {
			const auto hand = get_hand_no(i);

			if (hand.alive()) {
				if (callback(hand) == callback_result::ABORT) {
					return;
				}
			}
		}
	}

	augs::constant_size_vector<entity_id, 2> get_wielded_guns() const;
	augs::constant_size_vector<entity_id, 2> get_wielded_items() const;

	inventory_item_address get_address_from_root(const entity_id until = entity_id()) const;

	wielding_result make_wielding_transfers_for(const hand_selections_array) const;

	wielding_result swap_wielded_items() const;

private:
	template <class S, class I>
	callback_result for_each_contained_slot_and_item_recursive(
		S slot_callback, 
		I item_callback
	) const {
		const auto this_item_handle = *static_cast<const derived_handle_type*>(this);
		auto& cosm = this_item_handle.get_cosmos();

		if (const auto container = this_item_handle.template find<invariants::container>()) {
			for (const auto& s : container->slots) {
				const auto this_slot_id = inventory_slot_id(s.first, this_item_handle.get_id());
				const auto slot_callback_result = slot_callback(cosm[this_slot_id]);

				if (slot_callback_result == recursive_callback_result::ABORT) {
					return callback_result::ABORT;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
					continue;
				}
				else if (slot_callback_result == recursive_callback_result::CONTINUE_AND_RECURSE) {
					for (const auto& id : get_items_inside(this_item_handle, s.first)) {
						const auto child_item_handle = cosm[id];
						const auto item_callback_result = item_callback(child_item_handle);

						if (item_callback_result == recursive_callback_result::ABORT) {
							return callback_result::ABORT;
						}
						else if (item_callback_result == recursive_callback_result::CONTINUE_DONT_RECURSE) {
							continue;
						}
						else if (item_callback_result == recursive_callback_result::CONTINUE_AND_RECURSE) {
							if (child_item_handle.for_each_contained_slot_and_item_recursive(
								slot_callback,
								item_callback
							) == callback_result::ABORT) {
								return callback_result::ABORT;
							}
						}
						else {
							ensure(false && "bad recursive_callback_result");
						}
					}
				}
				else {
					ensure(false && "bad recursive_callback_result");
				}
			}
		}

		return callback_result::CONTINUE;
	}

public:

	template <class I>
	void for_each_contained_item_recursive(I&& item_callback) const {
		for_each_contained_slot_and_item_recursive(
			[](auto) { return recursive_callback_result::CONTINUE_AND_RECURSE; }, 
			std::forward<I>(item_callback)
		);
	}

	template <class S>
	void for_each_contained_slot_recursive(S&& slot_callback) const {
		for_each_contained_slot_and_item_recursive(
			std::forward<S>(slot_callback), 
			[](auto...) { return recursive_callback_result::CONTINUE_AND_RECURSE; }
		);
	}
};

template <class E>
std::optional<unsigned> inventory_mixin<E>::get_space_occupied() const {
	const auto& self = *static_cast<const E*>(this);

	if (const auto item = self.template find<components::item>()) {
		return item->get_charges() * self.template get<invariants::item>().space_occupied_per_charge;
	}

	return std::nullopt;
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_owning_transfer_capability() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	if (self.dead()) {
		return cosmos[entity_id()];
	}

	if (self.template has<components::item_slot_transfers>()) {
		return self;
	}

	const auto maybe_item = self.template find<components::item>();

	if (!maybe_item || cosmos[maybe_item->get_current_slot()].dead()) {
		return cosmos[entity_id()];
	}

	return cosmos[maybe_item->get_current_slot()].get_container().get_owning_transfer_capability();
}

template <class E>
bool inventory_mixin<E>::owning_transfer_capability_alive_and_same_as_of(const entity_id b) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();
	const auto this_capability = get_owning_transfer_capability();
	const auto b_capability = cosmos[b].get_owning_transfer_capability();

	return this_capability.alive() && b_capability.alive() && this_capability == b_capability;
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_primary_hand() const {
	const auto& self = *static_cast<const E*>(this);
	return self[slot_function::PRIMARY_HAND];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_secondary_hand() const {
	const auto& self = *static_cast<const E*>(this);
	return self[slot_function::SECONDARY_HAND];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_hand_no(const size_t index) const {
	const auto& self = *static_cast<const E*>(this);

	if (index == 0) {
		return get_primary_hand();
	}
	else if (index == 1) {
		return get_secondary_hand();
	}
	else {
		ensure(false && "bad hand index");
		return self.get_cosmos()[inventory_slot_id()];
	}
}

template <class E>
typename inventory_mixin<E>::generic_handle_type inventory_mixin<E>::get_if_any_item_in_hand_no(const size_t index) const {
	const auto& self = *static_cast<const E*>(this);
	const auto hand = self.get_hand_no(index);

	entity_id item;

	if (hand.alive()) {
		item = hand.get_item_if_any();
	}

	return self.get_cosmos()[item];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_first_free_hand() const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	inventory_slot_id target_slot;

	self.for_each_hand(
		[&](const auto hand) {
			if (hand.is_empty_slot()) {
				target_slot = hand;

				return callback_result::ABORT;
			}

			return callback_result::CONTINUE;
		}
	);

	return cosmos[target_slot];
}

template <class E>
typename inventory_mixin<E>::inventory_slot_handle_type inventory_mixin<E>::get_current_slot() const {
	const auto& self = *static_cast<const E*>(this);

	const auto maybe_item = self.template find<components::item>();

	if (maybe_item == nullptr) {
		return self.get_cosmos()[inventory_slot_id()];
	}

	return self.get_cosmos()[maybe_item->get_current_slot()];
}

template <class E>
inventory_item_address inventory_mixin<E>::get_address_from_root(const entity_id until) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	inventory_item_address output;
	inventory_slot_id current_slot = get_current_slot();

	while (cosmos[current_slot].alive()) {
		output.root_container = current_slot.container_entity;
		output.directions.push_back(current_slot.type);

		if (until == current_slot.container_entity) {
			break;
		}

		current_slot = cosmos[current_slot.container_entity].get_current_slot();
	}

	std::reverse(output.directions.begin(), output.directions.end());

	return output;
}

template <class E>
augs::constant_size_vector<entity_id, 2> inventory_mixin<E>::get_wielded_guns() const {
	const auto& self = *static_cast<const E*>(this);
	auto result = self.get_wielded_items();

	erase_if(
		result, 
		[&](const auto item) {
			return !self.get_cosmos()[item].template has<components::gun>();
		}
	);

	return result;
}

template <class E>
augs::constant_size_vector<entity_id, 2> inventory_mixin<E>::get_wielded_items() const {
	const auto& self = *static_cast<const E*>(this);
	augs::constant_size_vector<entity_id, 2> result;

	self.for_each_hand(
		[&](const auto hand) {
			const auto wielded = hand.get_item_if_any();

			if (wielded.alive()) {
				result.push_back(wielded);
			}

			return callback_result::CONTINUE;
		}
	);

	return result;
}

template <class E>
wielding_result inventory_mixin<E>::swap_wielded_items() const {
	const auto& self = *static_cast<const E*>(this);

	wielding_result result;

	const bool both_hands_available = self.get_hand_no(0).alive() && self.get_hand_no(1).alive();

	if (both_hands_available) {
		const auto in_primary = self.get_if_any_item_in_hand_no(0);
		const auto in_secondary = self.get_if_any_item_in_hand_no(1);

		auto& transfers = result.transfers;

		if (in_primary.alive() && in_secondary.alive()) {
			transfers = swap_slots_for_items(in_primary, in_secondary);
		}
		else if (in_primary.alive()) {
			transfers.push_back({ in_primary, self.get_secondary_hand() });
		}
		else if (in_secondary.alive()) {
			transfers.push_back({ in_secondary, self.get_primary_hand() });
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	return result;
}

template <class E>
wielding_result inventory_mixin<E>::make_wielding_transfers_for(const hand_selections_array selections) const {
	const auto& self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	wielding_result result;
	result.result = wielding_result::type::THE_SAME_SETUP;

	augs::constant_size_vector<item_slot_transfer_request, hand_count> holsters;
	augs::constant_size_vector<item_slot_transfer_request, hand_count> draws;

	for (size_t i = 0; i < selections.size(); ++i) {
		const auto hand = self.get_hand_no(i);

		if (hand.dead()) {
			continue;
		}

		const auto item_for_hand = cosmos[selections[i]];
		const auto item_in_hand = hand.get_item_if_any();

		const bool identical_outcome =
			(item_in_hand.dead() && item_for_hand.dead())
			|| item_in_hand == item_for_hand
		;

		if (identical_outcome) {
			continue;
		}

		if (item_in_hand.alive()) {
			const auto holstering_slot = self.determine_holstering_slot_for(item_in_hand);

			if (holstering_slot.dead()) {
				result.result = wielding_result::type::NO_SPACE_FOR_HOLSTER;
				break;
			}

			holsters.push_back({ item_in_hand, holstering_slot });
		}

		if (item_for_hand.alive()) {
			draws.push_back({ item_for_hand, hand });
		}

		result.result = wielding_result::type::SUCCESSFUL;
	}

	concatenate(result.transfers, holsters);
	concatenate(result.transfers, draws);

	return result;
}
