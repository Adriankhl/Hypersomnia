#pragma once
// disables the warning due to type name length exceeded
#pragma warning(disable : 4503)

#include "augs/templates/type_pair.h"
#include "augs/templates/type_list.h"
#include "augs/templates/type_matching_and_indexing.h"

#include "game/components/transform_component_declaration.h"
#include "game/components/sprite_component_declaration.h"
#include "game/components/polygon_component_declaration.h"

namespace invariants {
	struct gun;
	struct render;
	struct shape_polygon;
	struct shape_circle;
	struct trace;
	struct interpolation;
	struct flags;
	struct name;
	struct fixtures;
	struct rigid_body;
	struct container;
	struct item;
	struct missile;
	struct sentience;
	struct wandering_pixels;
	struct catridge;
	struct hand_fuse;
	struct explosive;
	struct movement;
	struct light;
	struct crosshair;
}

namespace components {
	struct animation;
	struct behaviour_tree;
	struct crosshair;
	struct missile;
	struct gun;
	struct movement;
	struct pathfinding;
	struct rigid_body;
	struct car;
	struct driver;
	struct specific_colliders_connection;
	struct item;
	struct force_joint;
	struct item_slot_transfers;
	struct trace;
	struct melee;
	struct sentience;
	struct attitude;
	struct processing;
	struct interpolation;
	struct light;
	struct wandering_pixels;
	struct motor_joint;
	struct hand_fuse;
	struct sender;
}

using assert_always_together = type_list<
	type_pair<invariants::gun, components::gun>,
	type_pair<invariants::trace, components::trace>,
	type_pair<invariants::interpolation, components::interpolation>,
	type_pair<invariants::rigid_body, components::rigid_body>,
	type_pair<invariants::item, components::item>,
	type_pair<invariants::missile, components::missile>,
	type_pair<invariants::sentience, components::sentience>,
	type_pair<invariants::wandering_pixels, components::wandering_pixels>,
	type_pair<invariants::hand_fuse, components::hand_fuse>,
	type_pair<invariants::movement, components::movement>,
	type_pair<invariants::light, components::light>,
	type_pair<invariants::crosshair, components::crosshair>
>;

using always_present_invariants = type_list<
	invariants::name,
	invariants::flags
>;

template <template <class...> class List>
using component_list_t = List<
	components::animation,
	components::behaviour_tree,
	components::crosshair,
	components::missile,
	components::gun,
	components::movement,
	components::pathfinding,
	components::rigid_body,
	components::specific_colliders_connection,
	components::transform,
	components::car,
	components::driver,
	components::item,
	components::force_joint,
	components::item_slot_transfers,
	components::trace,
	components::melee,
	components::sentience,
	components::attitude,
	components::interpolation,
	components::light,
	components::wandering_pixels,
	components::motor_joint,
	components::hand_fuse,
	components::sender
>;

template <template <class...> class List>
using invariant_list_t = List<
	invariants::name,
	invariants::flags,
	invariants::gun,
	invariants::render,
	invariants::shape_polygon,
	invariants::shape_circle,
	invariants::polygon,
	invariants::sprite,
	invariants::trace,
	invariants::interpolation,
	invariants::fixtures,
	invariants::rigid_body,
	invariants::container,
	invariants::item,
	invariants::missile,
	invariants::sentience,
	invariants::movement,
	invariants::wandering_pixels,
	invariants::catridge,
	invariants::explosive,
	invariants::hand_fuse,
	invariants::crosshair,
	invariants::light
>;

template <class... Types>
struct type_count {
	static const unsigned value = sizeof...(Types);
};

class cosmos;

constexpr unsigned COMPONENTS_COUNT = component_list_t<type_count>::value;
constexpr unsigned INVARIANTS_COUNT = invariant_list_t<type_count>::value;

template <class D>
static constexpr auto invariant_index_v = index_in_list_v<D, invariant_list_t<type_list>>;

template <class D>
static constexpr bool is_invariant_v = is_one_of_list_v<D, invariant_list_t<type_list>>;