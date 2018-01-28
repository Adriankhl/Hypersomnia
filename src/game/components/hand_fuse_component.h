#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/detail/view_input/sound_effect_input.h"

namespace components {
	struct hand_fuse {
		// GEN INTROSPECTOR struct components::hand_fuse
		augs::stepped_timestamp when_unpinned;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct hand_fuse {
		using implied_component = components::hand_fuse;

		// GEN INTROSPECTOR struct invariants::hand_fuse
		float fuse_delay_ms = 1000.f;

		sound_effect_input unpin_sound;
		sound_effect_input throw_sound;
		// END GEN INTROSPECTOR
	};
}
