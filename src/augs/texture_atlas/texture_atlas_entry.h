#pragma once
#include "augs/math/vec2.h"
#include "augs/pad_bytes.h"

namespace augs {
	struct texture_atlas_entry {
		xywh atlas_space;
		bool was_flipped = false;
		pad_bytes<3> pad;

		/*
			Each atlas entry caches its original size 
			to speed up calculations during drawing,
			even though it could be calculated via multiplying atlas_space by the real atlas size.
		*/

		vec2u cached_original_size_pixels = vec2u(0xdeadbeef, 0xdeadbeef);

		vec2u get_original_size() const {
			return cached_original_size_pixels;
		}

		vec2u get_size() const {
			return get_original_size();
		}

		vec2 get_atlas_space_uv(const vec2 entry_space) const;

		bool exists() const;
	};
}