#pragma once
#include "game/components/sprite_component.h"
#include "augs/misc/constant_size_vector.h"

namespace components {
	struct wandering_pixels {
		// GEN INTROSPECTOR struct components::wandering_pixels
		xywh reach = xywh(0.f, 0.f, 0.f, 0.f);
		rgba colorize = white;
		unsigned particles_count = 20u;
		// END GEN INTROSPECTOR
	};
}

namespace invariants {
	struct wandering_pixels {
		using implied_component = components::wandering_pixels;

		// GEN INTROSPECTOR struct invariants::wandering_pixels
		augs::constant_size_vector<invariants::sprite, 10> frames;
		float frame_duration_ms = 300.f;
		// END GEN INTROSPECTOR

		const invariants::sprite& get_face_after(const float passed_lifetime_ms) const {
			const auto frame_count = frames.size();
			const auto frame_num = static_cast<unsigned>(passed_lifetime_ms / frame_duration_ms) % frame_count;

			return frames[frame_num];
		}
	};
}