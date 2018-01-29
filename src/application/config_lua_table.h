#pragma once
#include <string>
#include <imgui/imgui.h>

#include "augs/pad_bytes.h"
#include "augs/math/vec2.h"
#include "augs/graphics/rgba.h"
#include "augs/image/font.h"

#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/game_intent_type.h"
#include "view/game_drawing_settings.h"
#include "view/audiovisual_state/world_camera.h"

// all settings structures stored by the config
#include "augs/window_framework/window_settings.h"
#include "augs/audio/audio_settings.h"
#include "game/debug_drawing_settings.h"

#include "view/game_gui/game_gui_intent_type.h"
#include "view/game_gui/elements/hotbar_settings.h"
#include "view/viewables/regeneration/content_regeneration_settings.h"
#include "view/audiovisual_state/systems/interpolation_settings.h"
#include "view/network/simulation_receiver_settings.h"

#include "test_scenes/test_scene_settings.h"

#include "application/debug_settings.h"
#include "application/session_settings.h"
#include "application/setups/main_menu_settings.h"
#include "application/setups/editor/editor_settings.h"
#include "application/app_intent_type.h"

enum class launch_type {
	// GEN INTROSPECTOR enum class launch_type
	MAIN_MENU,

	TEST_SCENE,
	EDITOR,

	LOCAL_DETERMINISM_TEST,

	DIRECTOR,

	ONLY_CLIENT,
	ONLY_SERVER,

	CLIENT_AND_SERVER,
	TWO_CLIENTS_AND_SERVER,

	COUNT
	// END GEN INTROSPECTOR
};

struct config_read_error : public std::runtime_error {
	explicit config_read_error(
		const augs::path_type& path,
		const std::string& what
	) 
		: std::runtime_error(
			std::string("There was a problem reading "  + path.string() + ".\n" + what)
		)
	{}
};

namespace sol {
	class state;
}

struct config_lua_table {
	config_lua_table() = default;
	config_lua_table(sol::state&, const augs::path_type& config_lua_path);

	// GEN INTROSPECTOR struct config_lua_table
	launch_type launch_mode = launch_type::TEST_SCENE;

	unit_tests_settings unit_tests;
	augs::window_settings window;
	augs::audio_settings audio;
	augs::audio_volume_settings audio_volume;
	debug_drawing_settings debug_drawing;
	hotbar_settings hotbar;
	world_camera_settings camera;
	game_drawing_settings drawing;
	interpolation_settings interpolation;
	simulation_receiver_settings simulation_receiver;
	content_regeneration_settings content_regeneration;
	main_menu_settings main_menu;

	game_intent_map game_controls;
	game_gui_intent_map game_gui_controls;
	app_intent_map app_controls;
	app_ingame_intent_map app_ingame_controls;
	
	ImGuiStyle gui_style;
	debug_settings debug;
	session_settings session;
	test_scene_settings test_scene;
	editor_settings editor;
	augs::font_loading_input gui_font;

#if TODO
	std::string connect_address;
	unsigned short connect_port = 0;
	unsigned short server_port = 0;
	unsigned short alternative_port = 0;

	unsigned jitter_buffer_ms = 0;
	unsigned client_commands_jitter_buffer_ms = 0;

	bool debug_randomize_entropies_in_client_setup = 0;
	unsigned debug_randomize_entropies_in_client_setup_once_every_steps = 0;

	augs::path_type db_path;
	augs::path_type survey_num_file_path;
	augs::path_type post_data_file_path;
	std::string last_session_update_link;

	augs::path_type director_input_scene_entropy_path;

	bool server_launch_http_daemon = 0;
	unsigned short server_http_daemon_port = 0;
	std::string server_http_daemon_html_file_path;
#endif

	// END GEN INTROSPECTOR

	launch_type get_launch_mode() const;
	input_recording_type get_input_recording_mode() const;

	void save(sol::state&, const augs::path_type& target_path) const;
};