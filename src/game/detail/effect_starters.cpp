#include "game/detail/effect_starters.h"
#include "game/messages/start_particle_effect.h"
#include "game/transcendental/logic_step.h"

void fire_and_forget_particle_effect(
	const particle_effect_input in,
	const logic_step step,
	const components::transform where,
	const entity_id homing_target = {}
) {
	messages::start_particle_effect msg;
	msg.effect = in;
	msg.positioning = where;

	step.post_message(msg);
}

void start_orbital_particle_effect(
	const particle_effect_input in,
	const logic_step step,  
	const orbital_chasing where,
	const entity_id homing_target = {}
) {
}

void start_orbital_particle_effect(
	const logic_step step,  
	const particle_effect_input in,
	const entity_id subject,
	const entity_id homing_target = {}
) {
	return start_orbital_particle_effect(step, in, { subject, {} }, homing_target);
}

void start_orbital_particle_effect(
	const logic_step step,  
	const particle_effect_input in,
	const components::transform where,
	const entity_id subject,
	const entity_id homing_target = {}
) {
	return start_orbital_particle_effect(step, in, { subject, where }, homing_target);
}
