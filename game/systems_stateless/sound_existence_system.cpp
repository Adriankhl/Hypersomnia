#include "sound_existence_system.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmos.h"
#include "game/messages/queue_destruction.h"

#include "game/detail/entity_scripts.h"

#include "game/components/damage_component.h"
#include "game/components/render_component.h"
#include "game/components/gun_component.h"
#include "game/components/fixtures_component.h"
#include "game/components/sentience_component.h"

#include "game/messages/collision_message.h"
#include "game/messages/gunshot_response.h"
#include "game/messages/create_particle_effect.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/damage_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/messages/health_event.h"
#include "game/messages/exhausted_cast_message.h"

bool components::sound_existence::is_activated(const const_entity_handle h) {
	return
		//h.get<components::dynamic_tree_node>().is_activated() && 
		h.get<components::processing>().is_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void components::sound_existence::activate(const entity_handle h) {
	if (is_activated(h)) {
		return;
	}

	auto& existence = h.get<components::sound_existence>();
	existence.time_of_birth = h.get_cosmos().get_timestamp();
	//h.get<components::dynamic_tree_node>().set_activated(true);
	h.get<components::processing>().enable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

void components::sound_existence::deactivate(const entity_handle h) {
	if (!is_activated(h)) {
		return;
	}

	//h.get<components::dynamic_tree_node>().set_activated(false);
	h.get<components::processing>().disable_in(processing_subjects::WITH_SOUND_EXISTENCE);
}

size_t components::sound_existence::random_variation_number_from_transform(const components::transform t) const {
	return time_of_birth.step + std::hash<vec2>()(t.pos);
}

float components::sound_existence::calculate_max_audible_distance() const {
	return input.effect.modifier.max_distance + input.effect.modifier.reference_distance;
}

void sound_existence_system::destroy_dead_sounds(const logic_step step) const {
	auto& cosmos = step.cosm;
	const auto timestamp = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_SOUND_EXISTENCE,
		[&](const auto it) {
			auto& existence = it.get<components::sound_existence>();

			const auto repetitions = existence.input.effect.modifier.repetitions;

			if (repetitions > -1 && (timestamp - existence.time_of_birth).step > existence.max_lifetime_in_steps * repetitions) {
				if (existence.input.delete_entity_after_effect_lifetime) {
					step.transient.messages.post(messages::queue_destruction(it));
				}
				else {
					components::sound_existence::deactivate(it);
				}
			}
		}
	);
}

void sound_existence_system::create_sounds_from_game_events(const logic_step step) const {
	const auto& collisions = step.transient.messages.get_queue<messages::collision_message>();
	const auto& gunshots = step.transient.messages.get_queue<messages::gunshot_response>();
	const auto& damages = step.transient.messages.get_queue<messages::damage_message>();
	const auto& swings = step.transient.messages.get_queue<messages::melee_swing_response>();
	const auto& healths = step.transient.messages.get_queue<messages::health_event>();
	const auto& exhausted_casts = step.transient.messages.get_queue<messages::exhausted_cast>();
	auto& cosmos = step.cosm;

	for (size_t i = 0; i < collisions.size(); ++i) {
		const auto& c = collisions[i];

		if (c.type == messages::collision_message::event_type::POST_SOLVE) {
			const auto subject = cosmos[c.subject];
			const auto collider = cosmos[c.collider];

			const auto& subject_fix = subject.get<components::fixtures>();
			const auto& collider_fix = collider.get<components::fixtures>();

			const auto& subject_coll = subject_fix.get_collider_data(c.subject_b2Fixture_index.collider_index);
			const auto& collider_coll = collider_fix.get_collider_data(c.collider_b2Fixture_index.collider_index);
			
			const auto sound_id = cosmos[subject_coll.material].collision_sound_matrix[collider_coll.material];

			const auto impulse = (c.normal_impulse + c.tangent_impulse) * subject_coll.collision_sound_gain_mult * collider_coll.collision_sound_gain_mult;

			const auto gain_mult = (impulse / 15.f) * (impulse / 15.f);
			const auto pitch_mult = impulse / 185.f;

			if (gain_mult > 0.01f) {
				sound_effect_input in;
				in.delete_entity_after_effect_lifetime = true;
				in.effect.modifier.pitch = 0.85f + pitch_mult;
				in.effect.modifier.gain = gain_mult;
				in.effect.id = sound_id;

				in.create_sound_effect_entity(cosmos, c.point, entity_id()).add_standard_components();
			}

			// skip the next, swapped collision message
			++i;
		}
	}

	for (const auto& g : gunshots) {
		for (const auto r : g.spawned_rounds) {
			const auto subject = cosmos[r];

			auto& damage = subject.get<components::damage>();

			sound_effect_input in;
			in.effect = damage.bullet_trace_sound_response;

			damage.trace_sound = in.create_sound_effect_entity(
				cosmos,
				g.muzzle_transform,
				r
			).add_standard_components();
		}

		{
			const auto subject = cosmos[g.subject];
			const auto& gun = subject.get<components::gun>();
			const auto gun_transform = subject.get_logic_transform();
			const auto owning_capability = subject.get_owning_transfer_capability();

			{
				sound_effect_input in;
				in.effect = gun.muzzle_shot_sound_response;
				in.direct_listener = owning_capability;

				in.create_sound_effect_entity(cosmos, subject.get_logic_transform(), entity_id()).add_standard_components();
			}

			{

				const auto cued_count = gun.num_last_bullets_to_trigger_low_ammo_cue;

				if (cued_count > 0) {
					const auto ammo_info = get_ammunition_information(subject);

					if (ammo_info.total_charges < cued_count) {
						sound_effect_input in;
						in.effect.id = assets::sound_buffer_id::LOW_AMMO_CUE;

						if (ammo_info.total_charges == cued_count - 1) {
							in.effect.modifier.gain = 0.65f;
						}

						in.direct_listener = owning_capability;

						in.create_sound_effect_entity(
							cosmos,
							gun_transform,
							entity_id()
						).add_standard_components();
					}
				}
			}
		}
	}

	for (const auto& h : healths) {
		const auto subject = cosmos[h.subject];
		const auto& sentience = subject.get<components::sentience>();

		sound_effect_input in;
		in.direct_listener = subject;

		if (h.target == messages::health_event::target_type::HEALTH) {
			if (h.special_result == messages::health_event::result_type::DEATH) {
				in.effect = sentience.death_sound_response;
			}
			else if (h.effective_amount > 0) {
				in.effect = sentience.health_decrease_sound_response;
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::PERSONAL_ELECTRICITY) {
			if (h.effective_amount > 0.f) {
				in.effect.id = assets::sound_buffer_id::EXPLOSION;
				in.effect.modifier.pitch = 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::PERSONAL_ELECTRICITY_DESTRUCTION) {
					in.effect.id = assets::sound_buffer_id::GREAT_EXPLOSION;
					in.effect.modifier.pitch = 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else if (h.target == messages::health_event::target_type::CONSCIOUSNESS) {
			if (h.effective_amount > 0.f) {
				in.effect.id = assets::sound_buffer_id::IMPACT;
				in.effect.modifier.pitch = 1.2f + h.effective_amount / 100.f;

				if (h.special_result == messages::health_event::result_type::LOSS_OF_CONSCIOUSNESS) {
					in.effect.id = assets::sound_buffer_id::DEATH;
					in.effect.modifier.pitch = 1.5f;
				}
			}
			else {
				continue;
			}
		}
		else {
			continue;
		}

		in.create_sound_effect_entity(
			cosmos, 
			subject.get_logic_transform(), 
			subject
		).add_standard_components();
	}

	for (const auto& d : damages) {
		if (d.inflictor_destructed) {
			const auto inflictor = cosmos[d.inflictor];
			
			sound_effect_input in;
			in.direct_listener = d.subject;
			in.effect = inflictor.get<components::damage>().destruction_sound_response;
			
			in.create_sound_effect_entity(
				cosmos, 
				d.point_of_impact, 
				entity_id()
			).add_standard_components();
		}
	}

	for (const auto& e : exhausted_casts) {
		const auto subject = cosmos[e.subject];

		sound_effect_input in;
		in.direct_listener = e.subject;
		in.effect.id = assets::sound_buffer_id::CAST_UNSUCCESSFUL;

		in.create_sound_effect_entity(
			cosmos, 
			e.transform, 
			entity_id()
		).add_standard_components();
	}
}