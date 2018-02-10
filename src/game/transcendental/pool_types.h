#pragma once
#include <tuple>

#include "augs/misc/declare_containers.h"
#include "game/transcendental/pool_size_type.h"
#include "game/organization/all_entity_types_declaration.h"

static constexpr bool statically_allocate_entities = STATICALLY_ALLOCATE_ENTITIES;

template <class E>
struct entity_solvable;

template <class T>
using make_entity_pool = std::conditional_t<
	statically_allocate_entities,
	augs::pool<entity_solvable<T>, of_size<T::statically_allocated_entities>::template make_constant_vector, cosmic_pool_size_type>,
	augs::pool<entity_solvable<T>, make_vector, cosmic_pool_size_type>
>;

using all_entity_pools = 
	replace_list_type_t<
		transform_types_in_list_t<
			all_entity_types,
			make_entity_pool
		>,
		std::tuple
	>
;
