#include "augs/templates/container_templates.h"
#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/components/position_copying_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/fixtures_component.h"

#include "view/viewables/all_viewables_declarations.h"
#include "view/viewables/particle_effect.h"
#include "view/viewables/particle_types.h"

#include "view/audiovisual_state/systems/particles_simulation_system.h"
#include "view/audiovisual_state/systems/interpolation_system.h"

void particles_simulation_system::erase_caches_for_dead_entities(const cosmos& new_cosmos) {
	std::vector<entity_id> to_erase;

	for (const auto it : per_entity_cache) {
		if (new_cosmos[it.first].dead()) {
			to_erase.push_back(it.first);
		}
	}

	for (const auto it : to_erase) {
		per_entity_cache.erase(it);
	}
}

particles_simulation_system::cache& particles_simulation_system::get_cache(const const_entity_handle id) {
	return per_entity_cache[id.get_id()];
}

void particles_simulation_system::add_particle(const render_layer l, const general_particle& p) {
	general_particles[l].push_back(p);
}

void particles_simulation_system::add_particle(const render_layer l, const animated_particle& p) {
	animated_particles[l].push_back(p);
}

void particles_simulation_system::add_particle(const render_layer l, const entity_id id, const homing_animated_particle& p) {
	homing_animated_particles[l][id].push_back(p);
}

void particles_simulation_system::advance_visible_streams_and_all_particles(
	camera_cone cone, 
	const cosmos& cosmos, 
	const particle_effects_map& manager,
	const augs::delta delta,
	const interpolation_system& interp
) {
	thread_local randomization rng;

	const auto dead_particles_remover = [](auto& container) {
		erase_if(container, [](const auto& a) { return a.is_dead(); });
	};

	for (auto& particle_layer : general_particles) {
		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds());
		}

		dead_particles_remover(particle_layer);
	}

	for (auto& particle_layer : animated_particles) {
		for (auto& p : particle_layer) {
			p.integrate(delta.in_seconds());
		}

		dead_particles_remover(particle_layer);
	}

	for (auto& particle_layer : homing_animated_particles) {
		erase_if(particle_layer, [&](auto& cluster) {
			const auto homing_target = cosmos[cluster.first];

			if (homing_target.alive()) {
				const auto homing_transform = cosmos[cluster.first].get_viewing_transform(interp);

				for (auto& p : cluster.second) {
					p.integrate(delta.in_seconds(), homing_transform.pos);
				}

				dead_particles_remover(cluster.second);

				return false;
			}
			
			return true;
		});
	}

	cone.visible_world_area *= 2.5f;

	auto update_target = [&](const unversioned_entity_id it_id) {
		const auto it = cosmos[it_id];

		auto& cache = get_cache(it);
		const auto& existence = it.get<components::particles_existence>();

		const bool should_rebuild_cache = cache.recorded_existence != existence;

		if (should_rebuild_cache) {
			cache.recorded_existence = existence;
			cache.emission_instances.clear();

			for (auto emission : manager.at(existence.input.effect.id).emissions) {
				emission.apply_modifier(existence.input.effect.modifier);

				cache.emission_instances.push_back(emission_instance());
				auto& new_emission_instance = *cache.emission_instances.rbegin();

				const auto var_v = rng.randval(emission.base_speed_variation);
				//LOG("V: %x", var_v);
				new_emission_instance.particle_speed.set(std::max(0.f, emission.base_speed.first - var_v / 2), emission.base_speed.second + var_v / 2);
				//LOG("Vl: %x Vu: %x", new_emission_instance.velocity.first, new_emission_instance.velocity.second);

				new_emission_instance.source_emission = emission;
				new_emission_instance.enable_streaming = true;
				new_emission_instance.stream_lifetime_ms = 0.f;
				new_emission_instance.angular_offset = rng.randval(emission.angular_offset);
				new_emission_instance.spread = rng.randval(emission.spread_degrees);
				new_emission_instance.particles_per_sec = rng.randval(emission.particles_per_sec);
				new_emission_instance.swing_spread = rng.randval(emission.swing_spread);
				new_emission_instance.swings_per_sec = rng.randval(emission.swings_per_sec);

				new_emission_instance.min_swing_spread = rng.randval(emission.min_swing_spread);
				new_emission_instance.min_swings_per_sec = rng.randval(emission.min_swings_per_sec);
				new_emission_instance.max_swing_spread = rng.randval(emission.max_swing_spread);
				new_emission_instance.max_swings_per_sec = rng.randval(emission.max_swings_per_sec);

				new_emission_instance.stream_max_lifetime_ms = rng.randval(emission.stream_lifetime_ms);
				new_emission_instance.stream_particles_to_spawn = rng.randval(emission.num_of_particles_to_spawn_initially);
				new_emission_instance.swing_speed_change = rng.randval(emission.swing_speed_change_rate);
				new_emission_instance.swing_spread_change = rng.randval(emission.swing_spread_change_rate);

				new_emission_instance.fade_when_ms_remaining = rng.randval(emission.fade_when_ms_remaining);

				new_emission_instance.randomize_spawn_point_within_circle_of_inner_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_inner_radius);
				new_emission_instance.randomize_spawn_point_within_circle_of_outer_radius = rng.randval(emission.randomize_spawn_point_within_circle_of_outer_radius);

				new_emission_instance.starting_homing_force = rng.randval(emission.starting_homing_force);
				new_emission_instance.ending_homing_force = rng.randval(emission.ending_homing_force);

				new_emission_instance.starting_spawn_circle_size_multiplier = rng.randval(emission.starting_spawn_circle_size_multiplier);
				new_emission_instance.ending_spawn_circle_size_multiplier = rng.randval(emission.ending_spawn_circle_size_multiplier);
			}
		}

		const auto transform = it.get_viewing_transform(interp) + existence.current_displacement;

		bool should_destroy = true;

		if (delta.in_seconds() > 0.0) {
			for (auto& instance : cache.emission_instances) {
				const auto stream_alivity_mult = 
					instance.stream_max_lifetime_ms == 0.f ? 1.f : instance.stream_lifetime_ms / instance.stream_max_lifetime_ms
				;

				const float stream_delta = std::min(delta.in_milliseconds(), instance.stream_max_lifetime_ms - instance.stream_lifetime_ms);
				const auto& emission = instance.source_emission;

				instance.stream_lifetime_ms += stream_delta;

				if (instance.stream_lifetime_ms > instance.stream_max_lifetime_ms) {
					continue;
				}

				auto new_particles_to_spawn_by_time = instance.particles_per_sec * (stream_delta / 1000.f);

				instance.stream_particles_to_spawn += new_particles_to_spawn_by_time;

				instance.swings_per_sec += rng.randval(-instance.swing_speed_change, instance.swing_speed_change);
				instance.swing_spread += rng.randval(-instance.swing_spread_change, instance.swing_spread_change);

				if (instance.max_swing_spread > 0) {
					augs::clamp(instance.swing_spread, instance.min_swing_spread, instance.max_swing_spread);
				}
				if (instance.max_swings_per_sec > 0) {
					augs::clamp(instance.swings_per_sec, instance.min_swings_per_sec, instance.max_swings_per_sec);
				}

				const int to_spawn = static_cast<int>(std::floor(instance.stream_particles_to_spawn));

				const auto segment_length = existence.distribute_within_segment_of_length;
				const vec2 segment_A = transform.pos + vec2().set_from_degrees(transform.rotation + 90).set_length(segment_length / 2);
				const vec2 segment_B = transform.pos - vec2().set_from_degrees(transform.rotation + 90).set_length(segment_length / 2);
				
				const auto homing_target_pos = cosmos[emission.homing_target].alive() ? cosmos[emission.homing_target].get_viewing_transform(interp).pos : vec2();

				for (int i = 0; i < to_spawn; ++i) {
					const float t = (static_cast<float>(i) / to_spawn);
					const float time_elapsed = (1.f - t) * delta.in_seconds();

					vec2 final_particle_position = augs::interp(segment_A, segment_B, rng.randval(0.f, 1.f));
					
					if (
						instance.randomize_spawn_point_within_circle_of_inner_radius > 0.f
						|| instance.randomize_spawn_point_within_circle_of_outer_radius > 0.f
						) {

						const auto size_mult = augs::interp(
							instance.starting_spawn_circle_size_multiplier,
							instance.ending_spawn_circle_size_multiplier,
							stream_alivity_mult
						);
						
						final_particle_position += rng.random_point_in_ring(
							size_mult * instance.randomize_spawn_point_within_circle_of_inner_radius,
							size_mult * instance.randomize_spawn_point_within_circle_of_outer_radius
						);
					}

					/* MSVC ICE fix */
					auto& _rng = rng;

					const auto spawner = [&](auto dummy) {
						using spawned_particle_type = decltype(dummy);

						return spawn_particle<spawned_particle_type>(
							_rng,
							instance.angular_offset,
							instance.particle_speed,
							final_particle_position,
							transform.rotation + instance.swing_spread * static_cast<float>(sin((instance.stream_lifetime_ms / 1000.f) * 2 * PI<float> * instance.swings_per_sec)),
							instance.spread,
							emission
						);
					};

					if (emission.get_definitions<general_particle>().size() > 0) {
						auto new_general = spawner(general_particle());
						new_general.integrate(time_elapsed);
						add_particle(emission.target_render_layer, new_general);
					}

					if (emission.get_definitions<animated_particle>().size() > 0)
					{
						auto new_animated = spawner(animated_particle());
						new_animated.integrate(time_elapsed);
						add_particle(emission.target_render_layer, new_animated);
					}

					if (emission.get_definitions<homing_animated_particle>().size() > 0)
					{
						auto new_homing_animated = spawner(homing_animated_particle());

						new_homing_animated.homing_force = augs::interp(
							instance.starting_homing_force,
							instance.ending_homing_force,
							stream_alivity_mult
						);

						new_homing_animated.integrate(time_elapsed, homing_target_pos);
						add_particle(emission.target_render_layer, emission.homing_target, new_homing_animated);
					}

					instance.stream_particles_to_spawn -= 1.f;
				}
			}
		}
	};

	cosmos.inferential.get<tree_of_npo_system>().for_each_visible_in_camera(
		update_target,
		cone,
		tree_of_npo_type::PARTICLE_EXISTENCES
	);
}
