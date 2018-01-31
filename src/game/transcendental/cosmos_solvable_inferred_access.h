#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/detail/inventory/inventory_slot_handle_declaration.h"

template <class, class>
class component_synchronizer;

template <class>
class physics_mixin;

class physics_system;
struct contact_listener;
class cosmic;

class cosmos_solvable_inferred_access {
	/*
		The following domains are free to change the cosmos_solvable::inferred,
		as they take proper precautions to keep state consistent.
	*/

	friend cosmic;

	template <class, class>
    friend class component_synchronizer;

	/* Special processors */
	friend physics_system;
	friend contact_listener;

	template <class>
	friend class physics_mixin;

	friend components::item;

	cosmos_solvable_inferred_access() {}
};
