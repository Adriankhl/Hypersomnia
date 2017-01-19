#include <algorithm>

#include "render_system.h"
#include "game/transcendental/entity_id.h"

#include "game/components/polygon_component.h"
#include "game/components/sprite_component.h"
#include "game/components/tile_layer_instance_component.h"
#include "game/components/particles_existence_component.h"

#include "game/messages/new_entity_message.h"
#include "game/messages/queue_destruction.h"

#include "game/transcendental/cosmos.h"
#include "game/enums/filters.h"

#include "game/detail/physics_scripts.h"
#include "augs/ensure.h"

bool render_system::render_order_compare(const const_entity_handle a, const const_entity_handle b) {
	const auto layer_a = a.get<components::render>().layer;
	const auto layer_b = a.get<components::render>().layer;

	return (layer_a == layer_b && layer_a == render_layer::CAR_INTERIOR) ? are_connected_by_friction(a, b) : layer_a < layer_b;
}

std::array<std::vector<const_entity_handle>, render_layer::COUNT> render_system::get_visible_per_layer(const std::vector<const_entity_handle>& entities) const {
	std::array<std::vector<entity_id>, render_layer::COUNT> layers;
	std::array<std::vector<const_entity_handle>, render_layer::COUNT> output;

	if (entities.empty()) {
		return output;
	}

	const auto& cosmos = entities[0].get_cosmos();

	for (const auto it : entities) {
		const auto layer = it.get<components::render>().layer;
		ensure(layer < static_cast<render_layer>(layers.size()));
		layers[layer].push_back(it);
	}

	auto& car_interior_layer = layers[render_layer::CAR_INTERIOR];

	if (car_interior_layer.size() > 1) {
		std::sort(car_interior_layer.begin(), car_interior_layer.end(), [&cosmos](const entity_id b, const entity_id a) {
			return are_connected_by_friction(cosmos[a], cosmos[b]);
		});
	}

	for (size_t i = 0; i < layers.size(); ++i) {
		output[i] = cosmos[layers[i]];
	}

	return output;
}

void render_system::draw_entities(
	const interpolation_system& interp,
	const float global_time_seconds,
	augs::vertex_triangle_buffer& output,
	const std::vector<const_entity_handle>& entities, 
	const camera_cone in_camera, 
	const renderable_drawing_type renderable_drawing_mode
) const {
	for (const auto e : entities) {
		for_each_type<components::polygon, components::sprite, components::tile_layer_instance>([&](auto T) {
			typedef decltype(T) renderable_type;

			if (e.has<renderable_type>()) {
				const auto& render = e.get<components::render>();
				const auto& renderable_transform = e.viewing_transform(interp, true);
				const auto& renderable = e.get<renderable_type>();

				render_system().draw_renderable(output, global_time_seconds, renderable, renderable_transform, render, in_camera, renderable_drawing_mode);
			}
		});
	}
}