#pragma once
#include <optional>
#include "augs/build_settings/platform_defines.h"

#include "game/transcendental/entity_handle_declaration.h"

#include "game/components/transform_component.h"
#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/components/wandering_pixels_component.h"

template <class entity_handle_type>
class spatial_properties_mixin {
public:
	bool has_independent_transform() const;
	void set_logic_transform(const components::transform t) const;

	template <class F, class... K>
	void access_independent_transform(
		F callback,
		K... keys	
	) const {
		using E = entity_handle_type;

		if constexpr(has_specific_entity_type_v<E>) {
			const auto handle = *static_cast<const entity_handle_type*>(this);
			auto& components = handle.get(keys...).components;

			if constexpr(E::template has<components::rigid_body>()) {
				if (!has_independent_transform()) {
					return;
				}

				callback(std::get<components::rigid_body>(components).physics_transforms);
			}
			else if constexpr(E::template has<components::transform>()) {
				callback(std::get<components::transform>(components));
			}
			else if constexpr(E::template has<components::wandering_pixels>()) {
				callback(std::get<components::wandering_pixels>(components).center);
			}
		}
		else {
			static_assert(always_false_v<E>, "Not implemented for non-specific handles.");
		}
	}

	components::transform get_logic_transform() const;
	std::optional<components::transform> find_logic_transform() const;

	template <class interpolation_system_type>
	std::optional<components::transform> find_viewing_transform(const interpolation_system_type& sys, const bool integerize = false) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);
		const auto& cosmos = handle.get_cosmos();

		if (const auto connection = handle.find_colliders_connection();
			connection && connection->owner != handle
		) {
			if (auto body_transform = sys.find_interpolated(cosmos[connection->owner])) {
				auto bt = *body_transform;

				if (integerize) {
					bt.pos.discard_fract();
				}

				auto displacement = connection->shape_offset;

				if (!displacement.pos.is_zero()) {
					displacement.pos.rotate(bt.rotation, vec2(0, 0));
				}

				return bt + displacement;
			}

			return std::nullopt;
		}
		
		return sys.find_interpolated(handle);
	}
	
	vec2 get_effective_velocity() const;

	std::optional<ltrb> find_aabb() const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto transform = handle.find_logic_transform()) {
			return find_aabb(*transform);
		}

		return std::nullopt;
	}

	template <class interpolation_system_type>
	std::optional<ltrb> find_aabb(const interpolation_system_type& interp) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto t = handle.find_viewing_transform(interp, true)) {
			return find_aabb(*t);
		}

		return std::nullopt;
	}

	std::optional<ltrb> find_aabb(const components::transform transform) const {
		const auto handle = *static_cast<const entity_handle_type*>(this);

		if (const auto* const sprite = handle.template find<invariants::sprite>();
			sprite != nullptr
		) {
			return sprite->get_aabb(transform);
		}

		if (const auto* const polygon = handle.template find<invariants::polygon>();
			polygon != nullptr
		) {
			return polygon->get_aabb(transform);
		}

		if (const auto* const wandering_pixels = handle.template find<components::wandering_pixels>();
			wandering_pixels != nullptr
		) {
			return xywh::center_and_size(
				transform.pos, 
				wandering_pixels->size
			);
		}

		/* TODO: Implement get_aabb for physical entities */
		ensure(!handle.template has<components::rigid_body>());
		ensure(nullptr == handle.template find<invariants::fixtures>());

		return std::nullopt;
	}


	/* Compatibility shortcuts. Their use is not recommended henceforth. */

	template <class... Args>
	auto get_aabb(Args&&... args) const {
		return *find_aabb(std::forward<Args>(args)...);
	}

	template <class... Args>
	auto get_viewing_transform(Args&&... args) const {
		return *find_viewing_transform(std::forward<Args>(args)...);
	}
};

template <class E>
components::transform spatial_properties_mixin<E>::get_logic_transform() const {
	const auto t = find_logic_transform();
	ensure(t.has_value());
	return *find_logic_transform();
}

template <class E>
std::optional<components::transform> spatial_properties_mixin<E>::find_logic_transform() const {
	const auto handle = *static_cast<const E*>(this);

#if MORE_LOGS
	LOG("Finding transform for %x.", handle);
#endif

	if (!handle) {
		return std::nullopt;
	}

	/*
		Since an alive owner body always implies that the entity has fixtures component,
		it is equivalent to the call of:

		const auto fixtures = owner.template find<invariants::fixtures>()

		But we would anyway need to get the owner body so we do it this way.
	*/

	if (const auto connection = handle.find_colliders_connection()) {
		const auto& cosmos = handle.get_cosmos();
		const auto owner = cosmos[connection->owner];

		if (const auto body = owner.template get<components::rigid_body>();
			body.is_constructed()
		) {
			const auto body_transform = body.get_transform();

			auto displacement = connection->shape_offset;

			if (!displacement.pos.is_zero()) {
				displacement.pos.rotate(body_transform.rotation, vec2(0, 0));
			}

			return body_transform + displacement;
		}
	}

	/*
		We only ever ask for transforms of fixtures, not rigid bodies. 
		We deem it pointless to ask for a position of a rigid body without fixtures.
		We will always assume that a potential rigid body comes with a fixtures component.	
		Otherwise there is anyway pretty much no reasonable way to simulate a fixtureless body.

		Thus let's have this commented out.

		if (const auto rigid_body = owner.template find<components::rigid_body>()) {
			ensure(false);

			// Should only ever happen for (entities without fixtures, but with rigid bodies), which is almost never

			return owner.template get<components::rigid_body>().get_transform();
		}
	*/

	/* The owner might have been dead due to the item being in a backpack, for example */
	if (const auto item = handle.template find<components::item>()) {
		const auto& cosmos = handle.get_cosmos();
		return cosmos[item->get_current_slot()].get_container().find_logic_transform();
	}

	if (const auto wandering_pixels = handle.template find<components::wandering_pixels>()) {
		return components::transform(wandering_pixels->center, 0);
	}

	if (const auto transform = handle.template find<components::transform>()) {
		return *transform;
	}	

	return std::nullopt;
}

template <class E>
vec2 spatial_properties_mixin<E>::get_effective_velocity() const {
	const auto handle = *static_cast<const E*>(this);

	if (const auto owner = handle.get_owner_of_colliders()) {
		return owner.template get<components::rigid_body>().get_velocity();
	}

	return {};
}

template <class E>
bool spatial_properties_mixin<E>::has_independent_transform() const {
	const auto handle = *static_cast<const E*>(this);
	const auto owner_body = handle.get_owner_of_colliders();

	if (owner_body.alive() && owner_body != handle) {
		/* 
			This body is connected to a different body, 
			therefore it has no independent transform. 
		*/
		return false;
	}

	return true;
}

template <class E>
void spatial_properties_mixin<E>::set_logic_transform(const components::transform t) const {
	const auto handle = *static_cast<const E*>(this);

	if (!has_independent_transform()) {
		return;
	}
	
	if (const auto rigid_body = handle.template find<components::rigid_body>()) {
		rigid_body.set_transform(t);
	}
	else if (const auto tr = handle.template find<components::transform>()) {
		*tr = t;
		// TODO: reinfer the npo cache where necessary
	}
	else if (const auto wp = handle.template find<components::wandering_pixels>()) {
		wp->center = t.pos;
		// TODO: reinfer the npo cache where necessary
	}
}
