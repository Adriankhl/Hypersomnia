#pragma once
#include "augs/math/camera_cone.h"
#include "augs/graphics/vertex.h"

#include "game/transcendental/entity_handle_declaration.h"

#include "view/game_gui/game_gui_intent_type.h"
#include "view/game_gui/elements/hotbar_settings.h"

class interpolation_system;
struct aabb_highlighter;

struct viewing_game_gui_context_dependencies {
	const interpolation_system& interpolation;
	const aabb_highlighter& world_hover_highlighter;
	const hotbar_settings hotbar;
	const double interpolation_ratio = 0.0;
	const game_gui_intent_map input_information;
	const camera_cone camera;
	const augs::drawer_with_default output;
};