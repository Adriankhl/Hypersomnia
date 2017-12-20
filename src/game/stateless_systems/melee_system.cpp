#include "melee_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"

#include "game/messages/intent_message.h"
#include "game/messages/melee_swing_response.h"
#include "game/detail/inventory/inventory_utils.h"

#include "game/components/melee_component.h"
#include "game/components/missile_component.h"
#include "game/components/fixtures_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

using namespace augs;

void components::melee::reset_weapon(const entity_handle e) {
	auto& m = e.get<components::melee>();
	m.reset_move_flags();
	m.current_state = melee_state::FREE;

	auto& d = e.get<components::missile>();
	d.damage_upon_collision = false;
}

void melee_system::consume_melee_intents(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();
	const auto& events = step.get_queue<messages::intent_message>();


	for (const auto& it : events) {
		/* 
			we search the whole intent message queue for events of interest;
			here we can even find gunshot requests for some distant enemy AI,
			therefore we need to filter out events we're interested in, and that would be
			melee-related intents and only these applied to an entity with a melee component
		*/
		auto* const maybe_melee = cosmos[it.subject].find<components::melee>();
		
		if (maybe_melee == nullptr) 
			continue;

		auto& melee = *maybe_melee;

		if (it.intent == game_intent_type::MELEE_PRIMARY_MOVE) {
			melee.primary_move_flag = it.was_pressed();
		}

		if (it.intent == game_intent_type::MELEE_SECONDARY_MOVE) {
			melee.secondary_move_flag = it.was_pressed();
		}
		
		if (it.intent == game_intent_type::MELEE_TERTIARY_MOVE) {
			melee.tertiary_move_flag = it.was_pressed();
		}
	}
}

void melee_system::initiate_and_update_moves(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto& delta = step.get_delta();
}