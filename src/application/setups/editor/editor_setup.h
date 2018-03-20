#pragma once
#include <future>
#include <map>

#include "augs/misc/timing/fixed_delta_timer.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_handle.h"
#include "game/detail/visible_entities.h"

#include "view/viewables/all_viewables_defs.h"
#include "view/viewables/viewables_loading_type.h"

#include "application/intercosm.h"

#include "application/debug_settings.h"

#include "application/setups/setup_common.h"

#include "application/setups/editor/editor_player.h"
#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_significant.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_settings.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/editor_recent_paths.h"
#include "application/setups/editor/editor_entity_selector.h"

#include "application/setups/editor/current_access_cache.h"

#include "application/setups/editor/gui/editor_history_gui.h"
#include "application/setups/editor/gui/editor_go_to_gui.h"
#include "application/setups/editor/gui/editor_all_entities_gui.h"
#include "application/setups/editor/gui/editor_common_state_gui.h"

struct config_lua_table;

namespace augs {
	class window;
	struct introspection_access;

	namespace event {
		struct change;
		struct state;
	}
}

struct editor_destructor_input {
	sol::state& lua;
};

class editor_setup : private current_access_cache<editor_setup> {
	using base = current_access_cache<editor_setup>;
	friend base;
	friend augs::introspection_access;

	double global_time_seconds = 0.0;

	editor_significant signi;
	editor_autosave autosave;
	editor_recent_paths recent;
	editor_settings settings;
	editor_player player;
	editor_entity_selector selector;

	editor_go_to_entity_gui go_to_entity_gui;

	// GEN INTROSPECTOR class editor_setup
	editor_history_gui history_gui;
	editor_all_entities_gui all_entities_gui = std::string("All entities");
	editor_all_entities_gui selected_entities_gui = std::string("Selected entities");
	editor_common_state_gui common_state_gui;
	// END GEN INTROSPECTOR

	std::optional<editor_popup> ok_only_popup;

	bool show_summary = true;

	editor_destructor_input destructor_input;

	const_entity_handle get_matching_go_to_entity() const;

	void on_folder_changed();
	void set_locally_viewed(const entity_id);
	void finish_rectangular_selection();

	void clear_id_caches();

	template <class F>
	void catch_popup(F&& callback) {
		try {
			callback();
		}
		catch (editor_popup p) {
			set_popup(p);
		}
	}

	template <class F>
	void try_to_open_new_folder(F&& new_folder_provider) {
		const auto new_index = signi.current_index + 1;

		signi.folders.reserve(signi.folders.size() + 1);
		signi.folders.emplace(signi.folders.begin() + new_index);

		auto& new_folder = signi.folders[new_index];

		base::refresh();

		try {
			new_folder_provider(new_folder);
			set_current(new_index);
		}
		catch (editor_popup p) {
			signi.folders.erase(signi.folders.begin() + new_index);
			set_popup(p);
		}

		base::refresh();
	}

	void play();
	void pause();

	cosmic_entropy total_collected_entropy;
	augs::fixed_delta_timer timer = { 5, augs::lag_spike_handling_type::DISCARD };

	std::future<std::optional<std::string>> open_folder_dialog;
	std::future<std::optional<std::string>> save_project_dialog;

	void set_popup(const editor_popup);
	
	using path_operation = intercosm_path_op;

	void open_folder_in_new_tab(path_operation);

	void save_current_folder();
	void save_current_folder_to(path_operation);

	void fill_with_minimal_scene(sol::state& lua);
	void fill_with_test_scene(sol::state& lua);

	void open_last_folders(sol::state& lua);

	void force_autosave_now() const;
	editor_command_input make_command_input();

	void load_gui_state();
	void save_gui_state();

public:
	static constexpr auto loading_strategy = viewables_loading_type::LOAD_ALL;
	static constexpr bool handles_window_input = true;
	static constexpr bool handles_escape = true;
	static constexpr bool has_additional_highlights = true;

	editor_setup(sol::state& lua);
	editor_setup(sol::state& lua, const augs::path_type& intercosm_path);
	
	~editor_setup();

	double get_audiovisual_speed() const;
	const cosmos& get_viewed_cosmos() const;
	real32 get_interpolation_ratio() const;
	entity_id get_viewed_character_id() const;
	const_entity_handle get_viewed_character() const;
	const all_viewables_defs& get_viewable_defs() const;

	void perform_custom_imgui(
		sol::state& lua,
		augs::window& owner,
		const bool in_direct_gameplay
	);

	void customize_for_viewing(config_lua_table& cfg) const;
	void apply(const config_lua_table& cfg);

	template <class... Callbacks>
	void advance(
		augs::delta frame_delta,
		Callbacks&&... callbacks
	) {
		global_time_seconds += frame_delta.in_seconds();

		if (!player.paused) {
			timer.advance(frame_delta *= player.speed);
		}

		auto steps = timer.extract_num_of_logic_steps(get_viewed_cosmos().get_fixed_delta());

		while (steps--) {
			total_collected_entropy.clear_dead_entities(work().world);

			work().advance(
				{ total_collected_entropy },
				std::forward<Callbacks>(callbacks)...
			);

			total_collected_entropy.clear();
		}
	}

	void control(const cosmic_entropy&);
	void accept_game_gui_events(const cosmic_entropy&);

	bool handle_input_before_imgui(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua
	);

	bool handle_input_before_game(
		const augs::event::state& common_input_state,
		const augs::event::change change,

		augs::window& window,
		sol::state& lua
	);

	std::optional<setup_escape_result> escape();
	bool confirm_modal_popup();

	void open(const augs::window& owner);
	void save(sol::state& lua, const augs::window& owner);
	void save_as(const augs::window& owner);
	void undo();
	void redo();

	void copy();
	void cut();
	void paste();

	void del();

	void play_pause();
	void stop();
	void prev();
	void next();

	void new_tab();
	void next_tab();
	void prev_tab();

	void close_folder();
	void close_folder(const folder_index i);

	void go_to_all();
	void go_to_entity();
	void select_all_entities();
	void reveal_in_explorer(const augs::window& owner);

	void unhover();
	bool is_editing_mode() const;
	std::optional<camera_cone> get_current_camera() const; 

	std::optional<ltrb> get_screen_space_rect_selection(vec2i screen_size, vec2i mouse_pos) const;

	template <class F>
	void for_each_selected_entity(F&& callback) const {
		if (anything_opened()) {
			selector.for_each_selected_entity(std::forward<F>(callback), view().selected_entities);
		}
	}

	std::unordered_set<entity_id> get_all_selected_entities() const;

	template <class F>
	void for_each_line(F callback) const {

	}

	template <class F>
	void for_each_highlight(F callback) const {
		if (anything_opened() && player.paused) {
			const auto& world = work().world;

			if (get_viewed_character().alive()) {
				auto color = settings.controlled_entity_color;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);

				callback(work().local_test_subject, color);
			}

			selector.for_each_highlight(
				callback,
				settings.entity_selector,
				world,
				view().selected_entities
			);

			if (const auto match = get_matching_go_to_entity()) {
				auto color = green;
				color.a += static_cast<rgba_channel>(augs::zigzag(global_time_seconds, 1.0 / 2) * 25);
				
				callback(match.get_id(), settings.matched_entity_color);
			}

			if (const auto hovered_guid = all_entities_gui.get_hovered_guid()) {
				if (const auto hovered = world[hovered_guid]) {
					callback(hovered.get_id(), settings.entity_selector.held_color);
				}
			}
		}
	}
};
