#pragma once
#include "augs/templates/maybe_const.h"
#include "augs/misc/basic_input_context.h"
#include "augs/graphics/vertex.h"
#include "augs/gui/dereferenced_location.h"
#include "augs/gui/basic_gui_context.h"
#include "augs/math/camera_cone.h"

#include "game/view/necessary_resources.h"
#include "game/enums/input_context_enums.h"
#include "game/transcendental/entity_handle.h"

#include "game/view/game_gui/elements/game_gui_context_dependencies.h"
#include "game/view/game_gui/elements/game_gui_element_location.h"
#include "game/view/game_gui/elements/viewing_game_gui_context_dependencies.h"

class game_gui_root;
class game_gui_system;
using game_gui_rect_tree = augs::gui::rect_tree<game_gui_element_location>;

struct character_gui;

template <bool is_const>
class basic_game_gui_context : public augs::gui::basic_context<
	game_gui_element_location, 
	is_const, 
	basic_game_gui_context<is_const>
> {
public:
	using base = augs::gui::basic_context<game_gui_element_location, is_const, basic_game_gui_context<is_const>>;
	using game_gui_system_ref = maybe_const_ref_t<is_const, game_gui_system>;
	using character_gui_ref = maybe_const_ref_t<is_const, character_gui>;
	using game_gui_root_ref = maybe_const_ref_t<is_const, game_gui_root>;
	using rect_world_ref = typename base::rect_world_ref;
	using tree_ref = typename base::tree_ref;

	basic_game_gui_context(
		const base b,

		game_gui_system_ref sys,
		const const_entity_handle handle,
		character_gui_ref char_gui,

		const game_gui_context_dependencies dependencies
	) : 
		base(b),

		sys(sys),
		handle(handle),
		char_gui(char_gui),

		dependencies(dependencies)
	{}

	game_gui_system_ref sys;
	const const_entity_handle handle;
	character_gui_ref char_gui;
	
	const game_gui_context_dependencies dependencies;

	operator basic_game_gui_context<true>() const {
		return {
			*static_cast<const base*>(this),

			sys,
			handle,
			char_gui,

			dependencies
		};
	}

	/* Boilerplate getters, pay no heed */

	auto& get_root() const {
		return sys.root;
	}

	auto get_root_id() const {
		return game_gui_root_in_context();
	}

	game_gui_system_ref get_game_gui_system() const {
		return sys;
	}

	const_entity_handle get_subject_entity() const {
		return handle;
	}

	character_gui_ref get_character_gui() const {
		return char_gui;
	}

	const cosmos& get_cosmos() const {
		return handle.get_cosmos();
	}

	const auto& get_gui_font() const {
		return dependencies.gui_font;
	}

	const auto& get_necessary_images() const {
		return dependencies.necessary_images;
	}

	const auto& get_game_images() const {
		return dependencies.game_images;
	}

	const auto& get_game_image_definitions() const {
		return dependencies.game_image_defs;
	}
};

using game_gui_context = basic_game_gui_context<false>;
using const_game_gui_context = basic_game_gui_context<true>;

class viewing_game_gui_context : public const_game_gui_context {
public:
	using base = const_game_gui_context;

	viewing_game_gui_context(
		const base b,
		const viewing_game_gui_context_dependencies in
	) :
		base(b),
		in(in)
	{}

	const viewing_game_gui_context_dependencies in;

	/* Boilerplate getters, pay no heed */

	const auto& get_output() const {
		return in.output;
	}

	const aabb_highlighter& get_world_hover_highlighter() const {
		return in.world_hover_highlighter;
	}

	const interpolation_system& get_interpolation_system() const {
		return in.interpolation;
	}

	camera_cone get_camera_cone() const {
		return in.camera;
	}

	auto get_hotbar_settings() const {
		return in.hotbar;
	}

	auto get_input_information() const {
		return in.input_information;
	}
};