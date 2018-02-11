#include "missile_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/messages/collision_message.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"

#include "game/detail/inventory/perform_transfer.h"
#include "game/detail/entity_scripts.h"

#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/transform_component.h"
#include "game/components/driver_component.h"
#include "game/components/fixtures_component.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/components/sentience_component.h"
#include "game/components/attitude_component.h"
#include "game/components/sender_component.h"
#include "game/components/explosive_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/physics/physics_scripts.h"

#include "game/assets/ids/sound_buffer_id.h"

#include "game/enums/filters.h"

#include "game/stateless_systems/sound_existence_system.h"

using namespace augs;

static void detonate_if_explosive(
	const logic_step step,
	const vec2 location,
	const const_entity_handle missile
) {
	if (const auto explosive = missile.find<invariants::explosive>()) {
		explosive->explosion.instantiate(step, location, entity_id());
	}
}

void missile_system::detonate_colliding_missiles(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::BEGIN_CONTACT || it.one_is_sensor) {
			continue;
		}

		const auto subject_handle = cosmos[it.subject];
		const auto missile_handle = cosmos[it.collider];

		if (auto missile = missile_handle.find<components::missile>()) {
			const auto& missile_def = missile_handle.get<invariants::missile>();
			const auto& sender = missile_handle.get<components::sender>();

			const bool bullet_colliding_with_any_subject_of_sender = sender.is_sender_subject(subject_handle);
			
			const bool should_send_damage =
				!bullet_colliding_with_any_subject_of_sender
				&& missile_def.damage_upon_collision
				&& missile->damage_charges_before_destruction > 0
			;

			if (should_send_damage) {
				const auto subject_of_impact = subject_handle.get_owner_of_colliders().get<components::rigid_body>();
				const auto subject_of_impact_mass_pos = subject_of_impact.get_mass_position(); 

				auto impact_velocity = missile_handle.get_effective_velocity();

				if (missile_def.impulse_upon_hit > 0.f) {
					auto considered_impulse = missile_def.impulse_upon_hit * missile->power_multiplier_of_sender;

					if (subject_handle.has<components::sentience>()) {
						if (!subject_handle.get<components::sentience>().get<electric_shield_perk_instance>().timing.is_enabled(now, delta)) {
							considered_impulse *= missile_def.impulse_multiplier_against_sentience;
						}
					}

					subject_of_impact.apply_force(vec2(impact_velocity).set_length(considered_impulse), it.point - subject_of_impact_mass_pos);
				}

				missile->saved_point_of_impact_before_death = it.point;

				const auto owning_capability = subject_handle.get_owning_transfer_capability();

				const bool is_victim_a_held_item = owning_capability.alive() && owning_capability != it.subject;

				messages::damage_message damage_msg;
				damage_msg.subject_b2Fixture_index = it.subject_b2Fixture_index;
				damage_msg.collider_b2Fixture_index = it.collider_b2Fixture_index;

				if (is_victim_a_held_item) {
					missile_def.pass_through_held_item_sound.start(
						step,
						sound_effect_start_input::fire_and_forget( { it.point, 0.f } ).set_listener(owning_capability)
					);
				}

				if (!is_victim_a_held_item && missile_def.destroy_upon_damage) {
					missile->damage_charges_before_destruction--;
					
					detonate_if_explosive(step, it.point, missile_handle);

					// delete only once
					if (missile->damage_charges_before_destruction == 0) {
						step.post_message(messages::queue_destruction(it.collider));
						damage_msg.inflictor_destructed = true;
					}
				}

				damage_msg.inflictor = it.collider;
				damage_msg.subject = it.subject;
				damage_msg.amount = missile_def.damage_amount * missile->power_multiplier_of_sender;
				damage_msg.impact_velocity = impact_velocity;
				damage_msg.point_of_impact = it.point;
				step.post_message(damage_msg);
			}
		}
	}
}

void missile_system::detonate_expired_missiles(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto now = cosmos.get_timestamp();
	const auto& delta = step.get_delta();

	cosmos.for_each_having<components::missile>(
		[&](const auto it) {
			using handle_type = std::decay_t<decltype(it)>;

			auto& missile = it.template get<components::missile>();
			auto& missile_def = it.template get<invariants::missile>();
		
			const bool already_detonated_in_this_step = 
				missile.damage_charges_before_destruction == 0
			;

			if (missile_def.constrain_lifetime && !already_detonated_in_this_step) {
				if (!missile.when_fired.was_set()) {
					missile.when_fired = now;
				}
				else {
					const auto fuse_delay_steps = static_cast<unsigned>(missile_def.max_lifetime_ms / delta.in_milliseconds());
					const auto when_detonates = missile.when_fired.step + fuse_delay_steps;

					if (/* should_already_detonate */ now.step >= when_detonates) {
						const auto current_pos = it.get_logic_transform().pos;

						missile.saved_point_of_impact_before_death = current_pos;
						detonate_if_explosive(step, current_pos, it);
						step.post_message(messages::queue_destruction(it));
					}
				}
			}

			const auto* const maybe_sender = it.template find<components::sender>();

			if (maybe_sender != nullptr && missile_def.homing_towards_hostile_strength > 0.f) {
				const auto sender_capability = cosmos[maybe_sender->capability_of_sender];
				const auto sender_attitude = 
					sender_capability && sender_capability.template has<components::attitude>() ? sender_capability : entity_handle::dead_handle(cosmos)
				;

				const auto particular_homing_target = cosmos[missile.particular_homing_target];
				
				const auto closest_hostile = 
					particular_homing_target.alive() ? particular_homing_target : cosmos[get_closest_hostile(it, sender_attitude, 250, filters::bullet())]
				;

				const auto current_velocity = it.template get<components::rigid_body>().get_velocity();

				it.set_logic_transform({ it.get_logic_transform().pos, current_velocity.degrees() });

				if (closest_hostile.alive()) {
					vec2 dirs[] = { current_velocity.perpendicular_cw(), -current_velocity.perpendicular_cw() };

					auto homing_vector = closest_hostile.get_logic_transform().pos - it.get_logic_transform().pos;

					if (dirs[1].dot(homing_vector) > dirs[0].dot(homing_vector)) {
						std::swap(dirs[0], dirs[1]);
					}

					it.template get<components::rigid_body>().apply_force(
						dirs[0].set_length(homing_vector.length()) * missile_def.homing_towards_hostile_strength
					);
				}
			}
		}
	);
}