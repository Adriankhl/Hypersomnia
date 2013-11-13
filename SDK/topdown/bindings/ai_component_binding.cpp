#pragma once
#include "stdafx.h"
#include "bindings.h"

#include "../components/ai_component.h"
#include "../systems/ai_system.h"

namespace bindings {
	luabind::scope _ai_component() {
		return
			luabind::class_<ai_system>("_ai_system")
			.def_readwrite("draw_cast_rays", &ai_system::draw_cast_rays)
			.def_readwrite("draw_triangle_edges", &ai_system::draw_triangle_edges)
			.def_readwrite("draw_discontinuities", &ai_system::draw_discontinuities)
			.def_readwrite("draw_memorised_walls", &ai_system::draw_memorised_walls)
			.def_readwrite("epsilon_ray_angle_variation", &ai_system::epsilon_ray_angle_variation)
			.def_readwrite("epsilon_distance_vertex_hit", &ai_system::epsilon_distance_vertex_hit)
			.def_readwrite("epsilon_threshold_obstacle_hit", &ai_system::epsilon_threshold_obstacle_hit)
			.def_readwrite("epsilon_max_segment_difference", &ai_system::epsilon_max_segment_difference)
			,

			luabind::class_<ai::visibility>("visibility")
			.def(luabind::constructor<>())
			.def_readwrite("filter", &ai::visibility::filter)
			.def_readwrite("square_side", &ai::visibility::square_side)
			.def_readwrite("color", &ai::visibility::color)
			.def_readwrite("postprocessing_subject", &ai::visibility::postprocessing_subject)
			.enum_("constants")
			[
				luabind::value("OBSTACLE_AVOIDANCE", ai::visibility::OBSTACLE_AVOIDANCE),
				luabind::value("DYNAMIC_PATHFINDING", ai::visibility::DYNAMIC_PATHFINDING)
			],
			
			luabind::class_<ai>("ai_component")
			.def(luabind::constructor<>())
			.def("start_pathfinding", &ai::start_pathfinding)
			.def("get_current_navigation_target", &ai::get_current_navigation_target)
			.def("clear_pathfinding_info", &ai::clear_pathfinding_info)
			.def("is_still_pathfinding", &ai::is_still_pathfinding)
			.def("add_request", &ai::add_request)
			.def("get_visibility", &ai::get_visibility)
			.def_readwrite("avoidance_width", &ai::avoidance_width)
			.def_readwrite("enable_backtracking", &ai::enable_backtracking)
			;
	}
}