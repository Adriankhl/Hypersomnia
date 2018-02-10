#pragma once
#include <optional>

#include "augs/misc/enum/enum_array.h"
#include "augs/drawing/flip.h"
#include "augs/texture_atlas/texture_atlas_entry.h"
#include "game/assets/ids/game_image_id.h"
#include "view/viewables/all_viewables_declarations.h"
#include "game/components/shape_polygon_component.h"

struct game_image_usage_as_button {
	// GEN INTROSPECTOR struct game_image_usage_as_button
	flip_flags flip;
	vec2 bbox_expander;
	// END GEN INTROSPECTOR
};

struct game_image_meta {
	// GEN INTROSPECTOR struct game_image_meta
	game_image_usage_as_button usage_as_button;
	// END GEN INTROSPECTOR
};

struct game_image_cache {
	game_image_cache(
		const game_image_loadables&,
		const game_image_meta&
	);

	vec2u original_image_size;
	convex_partitioned_shape partitioned_shape;

	vec2u get_size() const {
		return original_image_size;
	}
};

struct game_image_in_atlas {
	augs::texture_atlas_entry diffuse;
	augs::texture_atlas_entry neon_map;
	augs::texture_atlas_entry desaturated;

	operator augs::texture_atlas_entry() const {
		return diffuse;
	}

	vec2u get_size() const {
		return diffuse.get_original_size();
	}
};

struct loaded_game_image_caches : public asset_map<
	assets::game_image_id,
	game_image_cache
> {
	loaded_game_image_caches() = default;

	explicit loaded_game_image_caches(
		const game_image_loadables_map&,
		const game_image_metas_map&
	);
};

template <class E>
void add_shape_invariant_from_renderable(
	E& into,
	const loaded_game_image_caches& caches
) {
	static_assert(into.template has<invariants::shape_polygon>());

	if (const auto sprite = into.find<invariants::sprite>()) {
		const auto image_size = caches.at(sprite->tex).get_size();
		vec2 scale = sprite->get_size() / image_size;

		invariants::shape_polygon shape_polygon_def;

		shape_polygon_def.shape = caches.at(sprite->tex).partitioned_shape;
		shape_polygon_def.shape.scale(scale);

		into.set(shape_polygon_def);
	}

	if (const auto polygon = into.find<invariants::polygon>()) {
		std::vector<vec2> input;

		input.reserve(polygon->vertices.size());

		for (const auto& v : polygon->vertices) {
			input.push_back(v.pos);
		}

		invariants::shape_polygon shape_polygon_def;
		shape_polygon_def.shape.add_concave_polygon(input);

		into.set(shape_polygon_def);
	}
}
