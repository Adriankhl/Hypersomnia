#pragma once
#include "game/transcendental/entity_handle_declaration.h"
#include "game/transcendental/entity_handle.h"
#include "augs/graphics/vertex.h"

#include "augs/math/camera_cone.h"

class interpolation_system;

struct aabb_highlighter_drawing_input {
	const augs::drawer_with_default output;
	const const_entity_handle subject;
	const interpolation_system& interp;
	const camera_cone camera;
	const vec2 screen_size;
};

struct aabb_highlighter {
	float timer = 0.f;
	float cycle_duration_ms = 400.f;

	float base_gap = 2.f;
	float smallest_length = 8.f;
	float biggest_length = 16.f;
	float scale_down_when_aabb_no_bigger_than = 40.f;

	static bool is_hoverable(const const_entity_handle);

	void update(const augs::delta);
	void draw(const aabb_highlighter_drawing_input) const;
};