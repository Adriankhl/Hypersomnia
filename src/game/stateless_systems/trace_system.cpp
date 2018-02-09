#include "trace_system.h"
#include "augs/misc/randomization.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/components/trace_component.h"
#include "game/components/render_component.h"
#include "game/components/transform_component.h"
#include "game/components/sprite_component.h"
#include "game/components/missile_component.h"
#include "game/components/rigid_body_component.h"
#include "game/components/interpolation_component.h"

#include "game/messages/interpolation_correction_request.h"
#include "game/messages/queue_destruction.h"
#include "game/messages/will_soon_be_deleted.h"

void trace_system::lengthen_sprites_of_traces(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();

	cosmos.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.get<components::trace>();
			const auto& trace_def = t.get<invariants::trace>();

			vec2 surplus_multiplier;
			
			if (!trace.is_it_a_finishing_trace) {
				surplus_multiplier = trace.chosen_multiplier * trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms;
			}
			else {
				surplus_multiplier = (trace.chosen_multiplier + vec2(1, 1)) * (1.f - (trace.lengthening_time_passed_ms / trace.chosen_lengthening_duration_ms)) - vec2(1, 1);
			}

			const auto size_multiplier = trace_def.additional_multiplier + surplus_multiplier;

			trace.last_size_mult = size_multiplier; 
			trace.last_center_offset_mult = surplus_multiplier / 2.f;

			trace.lengthening_time_passed_ms += static_cast<float>(delta.in_milliseconds());
		}
	);
}

void trace_system::destroy_outdated_traces(const logic_step step) const {
	auto& cosmos = step.get_cosmos();

	cosmos.for_each_having<components::trace>(
		[&](const auto t) {
			auto& trace = t.get<components::trace>();

			if (trace.lengthening_time_passed_ms > trace.chosen_lengthening_duration_ms) {
				trace.lengthening_time_passed_ms = trace.chosen_lengthening_duration_ms;

				if (trace.is_it_a_finishing_trace) {
					step.post_message(messages::queue_destruction(t));
				}
			}
		}
	);
}

void trace_system::spawn_finishing_traces_for_deleted_entities(const logic_step step) const {
	auto& cosmos = step.get_cosmos();
	const auto& events = step.get_queue<messages::will_soon_be_deleted>();

	for (const auto& it : events) {
		const auto deleted_entity = cosmos[it.subject];

		const auto* const trace = deleted_entity.find<components::trace>();

		if (trace != nullptr
			&& !trace->is_it_a_finishing_trace
		) {
			const auto& trace_def = deleted_entity.get<invariants::trace>();

			const auto finishing_trace = cosmic::create_entity(
				cosmos, 
				trace_def.finishing_trace_flavour
			);
		
			auto transform_of_finishing = deleted_entity.get_logic_transform();

			if (const auto missile = deleted_entity.find<components::missile>()) {
				transform_of_finishing.pos = 
					missile->saved_point_of_impact_before_death
					- (deleted_entity.get<invariants::sprite>().get_size() / 2)
					.rotate(transform_of_finishing.rotation, vec2(0, 0))
				;
			}

			finishing_trace.set_logic_transform(transform_of_finishing);

			{
				auto& interp = finishing_trace.get<components::interpolation>();
				interp.place_of_birth = transform_of_finishing;
			}

			{
				auto& copied_trace = finishing_trace.get<components::trace>();
				copied_trace.lengthening_time_passed_ms = 0.f;
				copied_trace.chosen_lengthening_duration_ms /= 4;
				copied_trace.is_it_a_finishing_trace = true;
			}

			messages::interpolation_correction_request request;
			request.subject = finishing_trace;
			request.set_previous_transform_from = deleted_entity;

			step.post_message(request);
		}
	}
}

