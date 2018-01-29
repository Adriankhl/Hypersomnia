#include "game/transcendental/cosmos.h"
#include "game/assets/all_logical_assets.h"
#include "game/components/rigid_body_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/shape_polygon_component.h"
#include "game/components/sprite_component.h"

#include "game/enums/filters.h"
#include "game/transcendental/entity_handle.h"

namespace test_flavours {
	void add_standard_dynamic_body(entity_flavour& meta) {
		invariants::fixtures fixtures_def;
		invariants::rigid_body body_def;

		body_def.damping.linear = 6.5f;
		body_def.damping.angular = 6.5f;

		fixtures_def.filter = filters::dynamic_object();
		fixtures_def.density = 1;
		fixtures_def.material = assets::physical_material_id::METAL;

		meta.set(fixtures_def);
		meta.set(body_def);
	}

	void add_see_through_dynamic_body(entity_flavour& meta) {
		invariants::fixtures fixtures_def;
		invariants::rigid_body body_def;

		body_def.damping.linear = 6.5f;
		body_def.damping.angular = 6.5f;

		fixtures_def.filter = filters::see_through_dynamic_object();
		fixtures_def.density = 0.2f;
		fixtures_def.restitution = 0.5f;
		fixtures_def.material = assets::physical_material_id::METAL;

		meta.set(fixtures_def);
		meta.set(body_def);
	}

	void add_shell_dynamic_body(entity_flavour& meta) {
		invariants::fixtures fixtures_def;
		invariants::rigid_body body_def;

		body_def.damping.linear = 6.5f;
		body_def.damping.angular = 6.5f;

		fixtures_def.filter = filters::shell();
		fixtures_def.density = 1;
		fixtures_def.restitution = 1.4f;
		fixtures_def.density = 0.001f;
		fixtures_def.collision_sound_gain_mult = 100.f;
		fixtures_def.material = assets::physical_material_id::METAL;

		meta.set(fixtures_def);
		meta.set(body_def);
	}

	void add_standard_static_body(entity_flavour& meta) {
		invariants::fixtures fixtures_def;
		invariants::rigid_body body_def;

		body_def.damping.linear = 6.5f;
		body_def.damping.angular = 6.5f;

		body_def.body_type = rigid_body_type::STATIC;

		fixtures_def.filter = filters::dynamic_object();
		fixtures_def.density = 1;
		fixtures_def.material = assets::physical_material_id::METAL;

		meta.set(fixtures_def);
		meta.set(body_def);
	}
	
	void add_bullet_round_physics(entity_flavour& meta) {
		invariants::fixtures fixtures_def;
		invariants::rigid_body body_def;

		body_def.bullet = true;
		body_def.damping.angular = 0.f,
		body_def.damping.linear = 0.f,
		body_def.angled_damping = false;
		
		fixtures_def.filter = filters::bullet();
		fixtures_def.density = 1;
		fixtures_def.disable_standard_collision_resolution = true;
		fixtures_def.material = assets::physical_material_id::METAL;

		meta.set(fixtures_def);
		meta.set(body_def);
	}
}