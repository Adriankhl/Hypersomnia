#pragma once
#include <optional>
#include "augs/math/vec2.h"
#include "augs/build_settings/platform_defines.h"
#include "game/organization/all_components_declaration.h"
#include "game/detail/physics/colliders_connection.h"
#include "game/components/fixtures_component.h"

struct rigid_body_cache_info;

template <class derived_handle_type>
class physics_mixin {
public:
	static constexpr bool is_const = is_handle_const_v<derived_handle_type>;
	using generic_handle_type = basic_entity_handle<is_const>;

	generic_handle_type get_owner_friction_ground() const;
	
	auto& get_special_physics() const {
		const auto handle = *static_cast<const derived_handle_type*>(this);
		return handle.template get<components::rigid_body>().get_special();
	}

	std::optional<colliders_connection> find_colliders_connection() const;
	std::optional<colliders_connection> calc_colliders_connection() const;

	/* Shortcut for getting only the entity handle without shape offset */
	generic_handle_type get_owner_of_colliders() const;

	/* Assumes that the fixtures component is found. */
	real32 calc_density(
		const colliders_connection calculated_connection,
		const invariants::fixtures& def	
	) const;

	void infer_colliders() const;
};

template <class E>
typename physics_mixin<E>::generic_handle_type physics_mixin<E>::get_owner_friction_ground() const {
	const auto self = *static_cast<const E*>(this);
	return self.get_cosmos()[self.get_owner_of_colliders().get_special_physics().owner_friction_ground];
}

template <class E>
std::optional<colliders_connection> physics_mixin<E>::calc_colliders_connection() const {
	const auto self = *static_cast<const E*>(this);
	const auto& cosmos = self.get_cosmos();
	
	if (const auto overridden = self.template find<components::specific_colliders_connection>()) {
		return overridden->connection;
	}

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosmos[item->get_current_slot()]) {
			if (const auto topmost_container = self.calc_connection_to_topmost_container()) {
				if (auto topmost_container_connection = 
					cosmos[topmost_container->owner].calc_colliders_connection()
				) {
					// TODO: use attachment matrix
#if MORE_LOGS
					LOG("%x (item) owned by %x", self, cosmos[topmost_container_connection->owner]);
#endif
					return topmost_container_connection;
				}
			}

			return std::nullopt;
		}
	}

	if (self.template find<components::rigid_body>()) {
#if MORE_LOGS
		LOG("%x (body) owned by itself", self);
#endif
		return colliders_connection { self, {} };
	}

	return std::nullopt;
}

template <class E>
real32 physics_mixin<E>::calc_density(
	const colliders_connection calculated_connection,
	const invariants::fixtures& def	
) const {
	const auto self = *static_cast<const E*>(this);
	const auto& cosmos = self.get_cosmos();

	real32 density = def.density;

	if (const auto item = self.template find<components::item>()) {
		if (const auto slot = cosmos[item->get_current_slot()]) {
			density *= cosmos[item->get_current_slot()].calc_density_multiplier_due_to_being_attached();
		}
	}

	const auto owner_body = cosmos[calculated_connection.owner];

	if (const auto* const driver = owner_body.template find<components::driver>()) {
		if (cosmos[driver->owned_vehicle].alive()) {
			density *= driver->density_multiplier_while_driving;
		}
	}

	return density;
}

template <class E>
std::optional<colliders_connection> physics_mixin<E>::find_colliders_connection() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	if (const auto cache = cosmos.get_solvable_inferred().physics.find_colliders_cache(self)) {
		return cache->connection;
	}

	return calc_colliders_connection();
}

template <class E>
typename physics_mixin<E>::generic_handle_type physics_mixin<E>::get_owner_of_colliders() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	if (const auto connection = find_colliders_connection()) {
		return cosmos[connection->owner];
	}

	return cosmos[entity_id()];
}

template <class E>
void physics_mixin<E>::infer_colliders() const {
	const auto self = *static_cast<const E*>(this);
	auto& cosmos = self.get_cosmos();

	cosmos.get_solvable_inferred({}).physics.infer_cache_for_colliders(self);
}
