#include "destruction_system.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/messages/collision_message.h"
#include "game/messages/damage_message.h"

#include "game/components/fixtures_component.h"
#include "game/components/shape_circle_component.h"

void destruction_system::generate_damages_from_forceful_collisions(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto& events = step.get_queue<messages::collision_message>();

	for (const auto& it : events) {
		if (it.type != messages::collision_message::event_type::PRE_SOLVE || it.one_is_sensor) {
			continue;
		}
		
		const auto subject = cosmos[it.subject];
		const auto& fixtures = subject.get_def<definitions::fixtures>();

		const auto& data_indices = it.subject_b2Fixture_index;

		if (data_indices.is_set() && fixtures.is_destructible()) {
			//LOG("Destructible fixture was hit.");

			messages::damage_message damage_msg;
			damage_msg.subject_b2Fixture_index = it.subject_b2Fixture_index;
			damage_msg.collider_b2Fixture_index = it.collider_b2Fixture_index;

			damage_msg.inflictor = it.collider;
			damage_msg.subject = it.subject;
			damage_msg.amount = 0.f;
			damage_msg.impact_velocity = it.collider_impact_velocity;
			damage_msg.point_of_impact = it.point;

			step.post_message(damage_msg);
		}
	}
}

void destruction_system::apply_damages_and_split_fixtures(const logic_step step) const {

}