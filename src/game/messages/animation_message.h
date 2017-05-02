#pragma once
#include "message.h"
#include "game/assets/animation_id.h"

namespace messages {
	struct animation_message : message {
		enum action_type {
			INVALID,
			CONTINUE,
			START,
			PAUSE,
			STOP
		};

		action_type action = INVALID;

		assets::animation_id set_animation = assets::animation_id::INVALID;
		bool preserve_state_if_animation_changes = false;
		bool change_animation = false;
		bool change_speed = false;
		float speed_factor = 1.f;
		int animation_priority = 0;
	};
}
