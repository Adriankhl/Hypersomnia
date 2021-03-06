#pragma once
#include <variant>
#include "augs/gui/rect_world.h"

#include "application/gui/menu/locations/option_button_in_menu.h"
#include "application/gui/menu/locations/menu_root_in_context.h"

template <class E>
class option_button;

template <class Enum>
using menu_element_location = std::variant<
	option_button_in_menu<Enum, option_button<Enum>>,
	menu_root_in_context<Enum>
>;

template <class Enum>
using menu_rect_node = augs::gui::rect_node<menu_element_location<Enum>>;
