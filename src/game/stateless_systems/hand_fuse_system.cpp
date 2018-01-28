#include "hand_fuse_system.h"
#include "game/transcendental/entity_id.h"
#include "augs/log.h"

#include "game/transcendental/cosmos.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/hand_fuse_logic.h"
#include "game/detail/explosions.h"

#include "game/components/explosive_component.h"
#include "game/components/hand_fuse_component.h"
#include "game/messages/queue_destruction.h"

void hand_fuse_system::detonate_fuses(const logic_step step) {
	auto& cosmos = step.get_cosmos();
	const auto delta = step.get_delta();
	const auto now = cosmos.get_timestamp();

	cosmos.for_each(
		processing_subjects::WITH_HAND_FUSE,
		[&](const entity_handle it) {
			const auto& fuse = it.get<components::hand_fuse>();
			const auto when_unpinned = fuse.when_unpinned;

			if (when_unpinned.was_set()) {
				const auto& fuse_def = it.get<invariants::hand_fuse>();
				const auto fuse_delay_steps = static_cast<unsigned>(fuse_def.fuse_delay_ms / delta.in_milliseconds());

				if (const auto when_detonates = when_unpinned.step + fuse_delay_steps;
					now.step >= when_detonates
				) {
					if (const auto* const explosive = it.find<components::explosive>()) {
						/* Note: this assumes that an item inside a backpack returns a transform of the backpack. */
						const auto explosion_location = it.get_logic_transform();
						explosive->explosion.instantiate(step, explosion_location, entity_id());
						step.post_message(messages::queue_destruction(it));
					}
				}
			}
		}
	);
}