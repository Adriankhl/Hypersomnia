#include "echoes_of_the_higher_realms.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "game/components/sentience_component.h"
#include "game/detail/view_input/particle_effect_input.h"
#include "game/detail/view_input/sound_effect_input.h"
#include "game/detail/spells/spell_logic_input.h"
#include "game/detail/spells/spell_utils.h"
#include "game/stateless_systems/sentience_system.h"

#include "game/messages/health_event.h"

bool echoes_of_the_higher_realms_instance::are_additional_conditions_for_casting_fulfilled(const const_entity_handle subject) const {
	const auto& spell_data = std::get<echoes_of_the_higher_realms>(subject.get_cosmos().get_common_significant().spells);

	auto& consciousness = subject.get<components::sentience>().get<consciousness_meter_instance>();
	
	const bool would_heal_anything =
		consciousness.calc_damage_result(-spell_data.basic_healing_amount).effective < 0
	;

	return would_heal_anything;
}

void echoes_of_the_higher_realms_instance::perform_logic(const spell_logic_input in) {
	const auto subject = in.get_subject();
	const auto& spell_data = std::get<echoes_of_the_higher_realms>(subject.get_cosmos().get_common_significant().spells);
	const auto caster_transform = subject.get_logic_transform();

	ignite_cast_sparkles(spell_data, in.step, caster_transform, subject);
	play_cast_successful_sound(spell_data, in.step, caster_transform, subject);

	auto& consciousness = in.sentience.get<consciousness_meter_instance>();

	const auto result = consciousness.calc_damage_result(-spell_data.basic_healing_amount);

	messages::health_event event;
	event.subject = subject;
	event.point_of_impact = subject.get_logic_transform().pos;
	event.impact_velocity = { 0, -200 };
	event.effective_amount = result.effective;
	event.target = messages::health_event::target_type::CONSCIOUSNESS;
	event.ratio_effective_to_maximum = result.ratio_effective_to_maximum;

	sentience_system().consume_health_event(event, in.step);
}