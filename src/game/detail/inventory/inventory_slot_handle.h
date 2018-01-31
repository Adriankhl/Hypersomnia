#pragma once
#include <optional>
#include "augs/templates/maybe_const.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_id.h"

#include "game/enums/slot_function.h"
#include "game/enums/slot_physical_behaviour.h"

#include "game/detail/physics/colliders_connection.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_id.h"

#include "game/components/transform_component.h"

struct inventory_slot;

class cosmos;

template <class entity_handle_type>
class basic_inventory_slot_handle {
	static constexpr bool is_const = is_class_const_v<entity_handle_type>;

	using owner_reference = maybe_const_ref_t<is_const, cosmos>;

	/* Inventory slots are invariants, thus always const. */
	using slot_reference = const inventory_slot&;
	using slot_pointer = const inventory_slot*;

public:
	basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id);
	
	owner_reference get_cosmos() const;

	owner_reference owner;
	inventory_slot_id raw_id;

	void unset();

	slot_reference get() const;
	slot_reference operator*() const;
	slot_pointer operator->() const;

	bool alive() const;
	bool dead() const;

	bool can_contain(const entity_id) const;

	entity_handle_type get_item_if_any() const;
	entity_handle_type get_container() const;

	std::optional<colliders_connection> calculate_connection_until(entity_id = {}) const;

	entity_handle_type get_root_container() const;
	entity_handle_type get_root_container_until(const entity_id container_entity) const;

	bool is_child_of(const entity_id container_entity) const;

	const std::vector<entity_id>& get_items_inside() const;

	bool has_items() const;
	bool is_empty_slot() const;

	bool is_hand_slot() const;
	size_t get_hand_index() const;

	float calculate_density_multiplier_due_to_being_attached() const;

	unsigned calculate_local_space_available() const;
	unsigned calculate_real_space_available() const;

	bool is_physically_connected_until(const entity_id until_parent = entity_id()) const;

	inventory_slot_id get_id() const;
	operator inventory_slot_id() const;

	operator basic_inventory_slot_handle<typename entity_handle_type::const_type>() const {
		return { owner, raw_id };
	}

	operator bool() const {
		return alive();
	}
};

template <class E>
const auto& get_items_inside(const E h, const slot_function s) {
	return h.get_cosmos()[inventory_slot_id{s, h.get_id()}].get_items_inside();  
}

template <class E>
inline basic_inventory_slot_handle<E>::basic_inventory_slot_handle(owner_reference owner, const inventory_slot_id raw_id) : owner(owner), raw_id(raw_id) {}

template <class E>
inline typename basic_inventory_slot_handle<E>::owner_reference basic_inventory_slot_handle<E>::get_cosmos() const {
	return owner;
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_pointer basic_inventory_slot_handle<E>::operator->() const {
	return &get_container().template get<invariants::container>().slots.at(raw_id.type);
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_reference basic_inventory_slot_handle<E>::operator*() const {
	return *operator->();
}

template <class E>
inline typename basic_inventory_slot_handle<E>::slot_reference basic_inventory_slot_handle<E>::get() const {
	return *operator->();
}

template <class E>
inline bool basic_inventory_slot_handle<E>::alive() const {
	if (get_container().dead()) {
		return false;
	}

	const auto* const container = get_container().template find<invariants::container>();

	return container && container->slots.find(raw_id.type) != container->slots.end();
}

template <class E>
inline bool basic_inventory_slot_handle<E>::dead() const {
	return !alive();
}

template <class E>
inline inventory_slot_id basic_inventory_slot_handle<E>::get_id() const {
	return raw_id;
}

template <class E>
inline basic_inventory_slot_handle<E>::operator inventory_slot_id() const {
	return get_id();
}

template <class C>
auto subscript_handle_getter(C& cosm, const inventory_slot_id id) {
	return basic_inventory_slot_handle<basic_entity_handle<std::is_const_v<C>>>{ cosm, id };
}
template <class E>
void basic_inventory_slot_handle<E>::unset() {
	raw_id.unset();
}

template <class E>
bool basic_inventory_slot_handle<E>::is_hand_slot() const {
	return get_hand_index() != 0xdeadbeef;
}

template <class E>
std::size_t basic_inventory_slot_handle<E>::get_hand_index() const {
	std::size_t index = 0xdeadbeef;

	if (raw_id.type == slot_function::PRIMARY_HAND) {
		index = 0;
	}
	else if (raw_id.type == slot_function::SECONDARY_HAND) {
		index = 1;
	}

	return index;
}

template <class E>
bool basic_inventory_slot_handle<E>::has_items() const {
	return get_items_inside().size() > 0;
}

template <class E>
E basic_inventory_slot_handle<E>::get_item_if_any() const {
	return get_cosmos()[(has_items() ? (*this).get_items_inside()[0] : entity_id())];
}

template <class E>
bool basic_inventory_slot_handle<E>::is_empty_slot() const {
	return get_items_inside().size() == 0;
}

template <class E>
bool basic_inventory_slot_handle<E>::is_physically_connected_until(const entity_id until_parent) const {
	const bool should_item_here_keep_physical_body = get().makes_physical_connection();

	if (get_container() == until_parent) {
		return should_item_here_keep_physical_body;
	}

	const auto* const maybe_item = get_container().template find<components::item>();

	if (maybe_item) {
		//if (maybe_item->get_current_slot().get_container().alive() && maybe_item->get_current_slot().get_container() == until_parent)
		//	return should_item_here_keep_physical_body;
		//else 
		const auto slot = owner[maybe_item->get_current_slot()];

		if (slot.alive()) {
			return std::min(should_item_here_keep_physical_body, slot.is_physically_connected_until(until_parent));
		}
	}

	return should_item_here_keep_physical_body;
}

template <class E>
float basic_inventory_slot_handle<E>::calculate_density_multiplier_due_to_being_attached() const {
	const float density_multiplier = get().attachment_density_multiplier;

	if (const auto* const maybe_item = get_container().template find<components::item>()) {
		if (const auto slot = owner[maybe_item->get_current_slot()]) {
			if (slot->physical_behaviour == slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY) {
				return density_multiplier * slot.calculate_density_multiplier_due_to_being_attached();
			}

			return density_multiplier;
		}
	}

	return density_multiplier;
}

template <class E>
E basic_inventory_slot_handle<E>::get_root_container() const {
	const auto slot = get_container().get_current_slot();

	if (slot.alive()) {
		return slot.get_root_container();
	}

	return get_container();
}

template <class E>
std::optional<colliders_connection> basic_inventory_slot_handle<E>::calculate_connection_until(const entity_id until) const {
	const auto slot = get_container().get_current_slot();
	// const auto& cosmos = get_cosmos();

	if (slot.alive()) {
		if (slot->physical_behaviour == slot_physical_behaviour::DEACTIVATE_BODIES) {
			return std::nullopt;
		}

		ensure(slot->physical_behaviour == slot_physical_behaviour::CONNECT_AS_FIXTURE_OF_BODY); 
		/* TODO: calculate and sum attachment offsets from a matrix */
		/* TODO: return when until is found */
		return slot.calculate_connection_until();
	}

	return get_container().calculate_colliders_connection();
}

template <class E>
E basic_inventory_slot_handle<E>::get_root_container_until(const entity_id container_entity) const {
	const auto slot = get_container().get_current_slot();

	if (slot.alive() && slot.get_container() != container_entity) {
		return slot.get_root_container_until(container_entity);
	}

	return get_container();
}

template <class E>
bool basic_inventory_slot_handle<E>::is_child_of(const entity_id container_entity) const {
	return get_container() == container_entity || get_root_container_until(container_entity) == container_entity;
}

template <class E>
E basic_inventory_slot_handle<E>::get_container() const {
	return get_cosmos()[raw_id.container_entity];
}

template <class E>
unsigned basic_inventory_slot_handle<E>::calculate_real_space_available() const {
	const auto lsa = calculate_local_space_available();

	const auto* const maybe_item = get_container().template find<components::item>();

	if (maybe_item != nullptr && get_cosmos()[maybe_item->get_current_slot()].alive()) {
		return std::min(lsa, get_cosmos()[maybe_item->get_current_slot()].calculate_real_space_available());
	}

	return lsa;
}

template <class E>
bool basic_inventory_slot_handle<E>::can_contain(const entity_id id) const {
	if (dead()) {
		return false;
	}

	return query_containment_result(get_cosmos()[id], *this).transferred_charges > 0;
}

template <class E>
unsigned basic_inventory_slot_handle<E>::calculate_local_space_available() const {
	if (get().has_unlimited_space()) {
		return 1000000 * SPACE_ATOMS_PER_UNIT;
	}

	unsigned lsa = get().space_available;

	for (const auto e : get_items_inside()) {
		const auto occupied = calculate_space_occupied_with_children(get_cosmos()[e]);
		ensure(occupied <= lsa);
		lsa -= occupied;
	}

	return lsa;
}

template <class E>
const std::vector<entity_id>& basic_inventory_slot_handle<E>::get_items_inside() const {
	return get_cosmos().get_solvable_inferred().relational.get_items_of_slots().get_children_of(get_id());
}