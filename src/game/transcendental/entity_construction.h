#include "augs/misc/randomization.h"

#include "game/components/trace_component.h"
#include "game/components/interpolation_component.h"
#include "game/components/rigid_body_component.h"

#include "game/debug_utils.h"

template <class handle_type>
void construct_pre_inference(const handle_type h) {
	auto& cosmos = h.get_cosmos();

	if (const auto rigid_body = h.template find<components::rigid_body>()) {
		rigid_body.get_special().dropped_or_created_cooldown.set(200, cosmos.get_timestamp());
	}
}

template <class handle_type>
void construct_post_inference(const handle_type h) {
	auto& cosmos = h.get_cosmos();

	if (const auto interpolation = h.template find<components::interpolation>()) {
		if (const auto t = h.find_logic_transform()) {
			interpolation->place_of_birth = *t;
		}
		else {
			warning_other(h, "interpolation found but no transform could be found at time of birth");
		}
	}

	if (const auto trace = h.template find<components::trace>()) {
		auto rng = cosmos.get_fast_rng_for(h.get_id());
		trace->reset(*h.template find<invariants::trace>(), rng);
	}
}

std::string to_string(const std::wstring& val);

template <class handle_type>
void emit_warnings(const handle_type h) {
	if (const auto sprite = h.template find<invariants::sprite>()) {
		if (sprite->tex == assets::game_image_id::INVALID) {
			warning_unset_field(h, "invariants::sprite::tex");
		}

		if (sprite->size.is_zero()) {
			warning_unset_field(h, "invariants::sprite::size");
		}
	}

	if (const auto render = h.template find<invariants::render>()) {
		if (render->layer == render_layer::INVALID) {
			warning_unset_field(h, "invariants::render::layer");
		}
	}

	if (const auto polygon = h.template find<invariants::shape_polygon>()) {
		if (polygon->shape.convex_polys.empty()) {
			warning_unset_field(h, "invariants::shape_polygon::shape.convex_polys");
		}
	}
}

template <class handle_type>
void destruct_pre_deinference(const handle_type handle) {
	auto& cosmos = handle.get_cosmos();

	if (const auto container = handle.template find<invariants::container>()) {
		drop_from_all_slots(*container, handle, [](const auto&){});
	}
}