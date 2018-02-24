#pragma once
#include <cstddef>
#include "augs/pad_bytes.h"

#include "augs/misc/timing/stepped_timing.h"
#include "augs/misc/minmax.h"

#include "game/enums/gun_action_type.h"

#include "game/transcendental/entity_id.h"
#include "game/transcendental/step_declaration.h"
#include "game/transcendental/entity_flavour_id.h"

#include "game/components/render_component.h"

#include "game/assets/ids/recoil_player_id.h"
#include "game/assets/ids/sound_buffer_id.h"
#include "game/assets/recoil_player.h"

#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/view_input/particle_effect_input.h"

namespace augs {
	struct introspection_access;
}

namespace components {
	struct gun {
		// GEN INTROSPECTOR struct components::gun
		augs::stepped_timestamp when_last_fired;

		bool is_trigger_pressed = false;
		bool is_cocking_handle_being_pulled = false;

		pad_bytes<2> pad;

		float current_heat = 0.f;

		sound_effect_input firing_engine_sound;
		particle_effect_input muzzle_particles;
		augs::stepped_timestamp when_began_pulling_cocking_handle;

		recoil_player_instance recoil;
		// END GEN INTROSPECTOR

		static void load_next_round(
			const entity_id subject,
			const logic_step step
		);

		void set_cocking_handle_pulling(
			const bool enabled,
			const augs::stepped_timestamp now
		);
	};
}

namespace invariants {
	struct gun {
		using implied_component = components::gun;

		// GEN INTROSPECTOR struct invariants::gun
		float shot_cooldown_ms = 100.f;

		unsigned short num_last_bullets_to_trigger_low_ammo_cue = 0;
		gun_action_type action_mode = gun_action_type::INVALID;

		pad_bytes<1> pad;

		augs::minmax<float> muzzle_velocity;

		float damage_multiplier = 1.f;

		vec2 bullet_spawn_offset;

		augs::minmax<float> shell_velocity;
		augs::minmax<float> shell_angular_velocity;

		float shell_spread_degrees = 20.f;

		components::transform shell_spawn_offset;

		float gunshot_adds_heat = 0.05f;
		float maximum_heat = 1.f;
		float engine_sound_strength = 1.f;

		sound_effect_input muzzle_shot_sound;
		sound_effect_input low_ammo_cue_sound;

		float cocking_handle_pull_duration_ms = 500.f;

		constrained_entity_flavour_id<invariants::missile, components::sender> magic_missile_flavour;
		recoil_player_instance_def recoil;
		// END GEN INTROSPECTOR

		vec2 calc_muzzle_position(components::transform gun_transform) const;
		vec2 calc_barrel_center(components::transform gun_transform) const;
	};
}
