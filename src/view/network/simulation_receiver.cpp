#include "simulation_receiver.h"
#include "game/transcendental/cosmic_delta.h"
#include "augs/network/network_client.h"

#include "game/components/driver_component.h"
#include "game/components/flags_component.h"
#include "game/components/crosshair_component.h"
#include "game/components/gun_component.h"
#include "game/components/item_component.h"

#include "view/audiovisual_state/systems/interpolation_system.h"
#include "view/audiovisual_state/systems/past_infection_system.h"

#include "view/network/network_commands.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"

#include "game/detail/inventory/inventory_slot_handle.h"

void simulation_receiver::acquire_next_packaged_step(const step_packaged_for_network& step) {
	jitter_buffer.acquire_new_command(step);
}

void simulation_receiver::predict_intents_of_remote_entities(
	guid_mapped_entropy& adjusted_entropy, 
	const entity_id locally_controlled_entity, 
	const cosmos& predicted_cosmos
) {
	predicted_cosmos.for_each(
		processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS,
		[&](const auto e) {
			const bool is_locally_controlled_entity = e == locally_controlled_entity;
			
			if (is_locally_controlled_entity) {
				return;
			}

			for (const auto g_id : e.get_wielded_guns()) {
				const auto g = predicted_cosmos[g_id];

				if (g.get<components::gun>().is_trigger_pressed) {
					const auto current_slot = g.get_current_slot();

					const auto hand_index = current_slot.get_hand_index();

					if (current_slot.alive() && hand_index != 0xdeadbeef) {
						game_intent release_intent;

						if (hand_index == 0) {
							release_intent.is_pressed = false;
							release_intent.intent = intent_type::CROSSHAIR_PRIMARY_ACTION;
						}
						else if (hand_index == 1) {
							release_intent.is_pressed = false;
							release_intent.intent = intent_type::CROSSHAIR_PRIMARY_ACTION;
						}
						else {
							ensure(false && "bad hand index");
						}

						if (release_intent.is_set()) {
							adjusted_entropy.intents_per_entity[e.get_guid()].push_back(release_intent);
						}
					}
				}
			}
		}
	);
}

steps_unpacking_result simulation_receiver::unpack_deterministic_steps(
	cosmos& referential_cosmos, 
	cosmos& last_delta_unpacked
) {
	steps_unpacking_result result;

	auto new_commands = jitter_buffer.buffer;
	jitter_buffer.buffer.clear();

	for (size_t i = 0; i < new_commands.size(); ++i) {
		auto& new_command = new_commands[i];

		//if (new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY_WITH_HEARTBEAT) {
		//	cosmic_delta::decode(last_delta_unpacked, new_command.delta);
		//	referential_cosmos = last_delta_unpacked;
		//
		//	entropies_to_simulate.clear();
		//	reconciliate_predicted = true;
		//}
		//else
			ensure(new_command.step_type == step_packaged_for_network::type::NEW_ENTROPY);

		step_to_simulate sim;
		sim.reinfer = new_command.shall_reinfer;
		sim.entropy = new_command.entropy;

		result.entropies_to_simulate.emplace_back(sim);

		const auto& actual_server_step = sim.entropy;
		const auto& predicted_server_step = predicted_step_entropies.front();

		if (sim.reinfer || actual_server_step != predicted_server_step) {
			result.reconciliate_predicted = true;
		}

		if (new_command.next_client_commands_accepted) {
			ensure(predicted_step_entropies.size() > 0);
			predicted_step_entropies.erase(predicted_step_entropies.begin());
		}
	}

	return result;
}

misprediction_candidate_entry simulation_receiver::acquire_potential_misprediction(
	const const_entity_handle e
) const {
	misprediction_candidate_entry candidate;
	
	if (e.has_logic_transform()) {
		candidate.transform = e.get_logic_transform();
	}

	candidate.id = e.get_id();

	return candidate;
}

std::vector<misprediction_candidate_entry> simulation_receiver::acquire_potential_mispredictions(
	const std::unordered_set<entity_id>& unpredictables_infected, 
	const cosmos& predicted_cosmos_before_reconciliation
) const {
	std::vector<misprediction_candidate_entry> potential_mispredictions;
	
	const auto& cosmos = predicted_cosmos_before_reconciliation;
	
	potential_mispredictions.reserve(
		cosmos.get_count_of(processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS) + unpredictables_infected.size()
	);

	cosmos.for_each(
		processing_subjects::WITH_ENABLED_PAST_CONTAGIOUS,
		[&](const auto e) {
			potential_mispredictions.push_back(acquire_potential_misprediction(e));
		}
	);

	for (const auto& e : unpredictables_infected) {
		if (cosmos[e].alive()) {
			potential_mispredictions.push_back(acquire_potential_misprediction(cosmos[e]));
		}
	}

	return potential_mispredictions;
}

void simulation_receiver::drag_mispredictions_into_past(
	const simulation_receiver_settings& settings,
	interpolation_system& interp,
	past_infection_system& past, 
	const cosmos& predicted_cosmos, 
	const std::vector<misprediction_candidate_entry>& mispredictions
) const {
	for (const auto e : mispredictions) {
		const const_entity_handle reconciliated_entity = predicted_cosmos[e.id];
		
		const bool identity_matches = reconciliated_entity.alive() && reconciliated_entity.has_logic_transform();

		if (!identity_matches) {
			continue;
		}

		const auto& reconciliated_transform = reconciliated_entity.get_logic_transform();
		const bool is_contagious_agent = reconciliated_entity.get_flag(entity_flag::IS_PAST_CONTAGIOUS);
		const bool should_smooth_rotation = !is_contagious_agent || predicted_cosmos[reconciliated_entity.get<components::driver>().owned_vehicle].alive();

		auto& interp_data = interp.get_cache_of(reconciliated_entity);

		const bool shouldnt_smooth = reconciliated_entity.has<components::crosshair>();
		bool misprediction_detected = false;

		const float num_predicted_steps = static_cast<float>(predicted_step_entropies.size());

		if (!shouldnt_smooth && (reconciliated_transform.pos - e.transform.pos).length_sq() > 1.f) {
			interp_data.positional_slowdown_multiplier = std::max(1.f, settings.misprediction_smoothing_multiplier * num_predicted_steps);
			misprediction_detected = true;
		}

		if (should_smooth_rotation && std::abs(reconciliated_transform.rotation - e.transform.rotation) > 1.f) {
			interp_data.rotational_slowdown_multiplier = std::max(1.f, settings.misprediction_smoothing_multiplier * num_predicted_steps);
			misprediction_detected = true;
		}

		if (identity_matches || (!misprediction_detected && !is_contagious_agent)) {
			past.uninfect(reconciliated_entity);
		}
	}
}
