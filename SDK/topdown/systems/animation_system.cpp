#include "stdafx.h"
#include "animation_system.h"
#include "entity_system/world.h"
#include "../messages/animate_message.h"

using namespace messages;
using namespace resources;

void animation_system::process_events(world& owner) {
	auto events = owner.get_message_queue<animate_message>();

	for (auto it : events) {
		auto ptr = it.subject->find<components::animate>();
		if (!ptr) continue; auto& animate = *ptr;

		if (it.animation_priority >= animate.current_priority || animate.current_state == components::animate::state::PAUSED) {
			if (it.change_speed)
				animate.speed_factor = it.speed_factor;

			if (it.change_animation) {
				resources::animation* new_instance = nullptr;
				if (it.set_animation)
					new_instance = it.set_animation;
				else {
					auto found_animation = animate.available_animations->get_raw().find(it.animation_type);
					if (found_animation != animate.available_animations->get_raw().end()) {
						new_instance = (*found_animation).second;
					}
				}

				if (new_instance != animate.current_animation) {
					animate.current_animation = new_instance;
					if (!it.preserve_state_if_animation_changes) {
						animate.set_current_frame(0, it.subject);
						animate.current_ms = 0.f;
					}
					else {
						/* update callback */
						animate.set_current_frame(animate.get_current_frame(), it.subject);
					}
				}
			}
			
			animate.current_priority = it.animation_priority;

			switch (it.message_type) {
			case animate_message::type::PAUSE:
				if (animate.current_state != components::animate::state::PAUSED) {
					animate.paused_state = animate.current_state;
					animate.current_state = components::animate::state::PAUSED;
				}
				break;
			case animate_message::type::STOP:
				animate.paused_state = components::animate::state::INCREASING;
				animate.current_state = components::animate::state::PAUSED;
				animate.set_current_frame(0, it.subject);
				animate.current_ms = 0.f;
				break;
			case animate_message::type::START:
				animate.current_state = components::animate::state::INCREASING;
				animate.set_current_frame(0, it.subject);
				animate.current_ms = 0.f;
				break;
			case animate_message::type::CONTINUE:
				if (animate.current_state == components::animate::state::PAUSED) {
					animate.current_state = animate.paused_state;
				}
				break;
			default: break;
			}
		}
	}
}

void components::animate::set_current_frame(unsigned number, augmentations::entity_system::entity* subject) {
	current_frame = number;
	auto callback = current_animation->frames[current_frame].callback;

	if (callback) {
		try {
			luabind::call_function<void>(callback, subject);
		}
		catch (std::exception compilation_error) {
			std::cout << compilation_error.what() << '\n';
		}
	}
}

void animation_system::process_entities(world& owner) {
	float delta = static_cast<float>(animation_timer.extract<std::chrono::milliseconds>());

	for (auto it : targets) {
		auto& animate = it->get<components::animate>();
		auto& render = it->get<components::render>();
		if (animate.current_animation == nullptr) continue;

		auto& animation = *animate.current_animation;
		if (animation.frames.empty()) continue;

		if (animate.current_state != components::animate::state::PAUSED) {
			animate.current_ms += delta * animate.speed_factor;

			while (true) {
				float frame_duration = animation.frames[animate.get_current_frame()].duration_milliseconds;

				if (animate.current_ms > frame_duration) {
					animate.current_ms -= frame_duration;

					if (animation.loop_mode == animation::loop_type::INVERSE) {

						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.get_current_frame() < animation.frames.size() - 1) animate.increase_frame(it);
							else {
								animate.decrease_frame(it);
								animate.current_state = components::animate::state::DECREASING;
							}
						}

						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.get_current_frame() > 0) animate.decrease_frame(it);
							else {
								animate.increase_frame(it);
								animate.current_state = components::animate::state::INCREASING;
							}
						}
					}

					else if (animation.loop_mode == animation::loop_type::REPEAT) {
						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.get_current_frame() < animation.frames.size() - 1)
								animate.increase_frame(it);
							else animate.set_current_frame(0, it);
						}
						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.get_current_frame() > 0) animate.decrease_frame(it);
							else animate.set_current_frame(animation.frames.size() - 1, it);
						}
					}

					else if (animation.loop_mode == animation::loop_type::NONE) {
						if (animate.current_state == components::animate::state::INCREASING) {
							if (animate.get_current_frame() < animation.frames.size() - 1)
								animate.increase_frame(it);
							else animate.current_state = components::animate::state::PAUSED;
						}
						else if (animate.current_state == components::animate::state::DECREASING) {
							if (animate.get_current_frame() > 0) animate.decrease_frame(it);
							else animate.current_state = components::animate::state::PAUSED;
						}
					}
				}
				else break;
			}
		}

		render.model = &animation.frames[animate.get_current_frame()].model;
	}
}
