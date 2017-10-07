#include "augs/misc/lua_readwrite.h"
#include "augs/misc/http_requests.h"
#include "augs/misc/standard_actions.h"

#include "augs/filesystem/file.h"
#include "augs/templates/string_templates.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"
#include "augs/audio/sound_source.h"

#include "augs/gui/text/caret.h"
#include "augs/gui/text/printer.h"
#include "augs/drawing/drawing.h"

#include "game/assets/all_logical_assets.h"

#include "game/organization/all_component_includes.h"
#include "view/network/step_packaged_for_network.h"
#include "game/transcendental/cosmos.h"
#include "game/transcendental/logic_step.h"
#include "game/transcendental/cosmic_movie_director.h"
#include "game/organization/all_messages_includes.h"
#include "game/transcendental/data_living_one_step.h"

#include "game/detail/visible_entities.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"

#include "application/config_lua_table.h"

#include "application/setups/main_menu_setup.h"

#include "application/gui/menu/menu_root.h"
#include "application/gui/menu/menu_context.h"
#include "application/gui/menu/appearing_text.h"
#include "application/gui/menu/creators_screen.h"

#include "generated/introspectors.h"

using namespace augs::event::keys;
using namespace augs::gui::text;
using namespace augs::gui;

void main_menu_setup::customize_for_viewing(config_lua_table& config) const {
	const auto previous_sfx_volume = config.audio_volume.sound_effects;
	augs::read(menu_config_patch, config);
	
	/* Treat new volume as a multiplier */

	config.audio_volume.sound_effects *= previous_sfx_volume;
}

void main_menu_setup::apply(const config_lua_table& config) {
	menu_theme_source.set_gain(config.audio_volume.music);
}

void main_menu_setup::launch_creators_screen() {
#if TODO
	if (credits_actions.is_complete()) {
		creators = creators_screen();
		creators.setup(textes_style, style(assets::font_id::GUI_FONT, { 0, 180, 255, 255 }), window.get_screen_size());

		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 170, 500.f)));
		creators.push_into(credits_actions);
		credits_actions.push_blocking(act(new augs::tween_value_action<rgba_channel>(fade_overlay_color.a, 20, 500.f)));
	}
#endif
}

void main_menu_setup::query_latest_news(const std::string& url) {
	if (latest_news.valid()) {
		latest_news.wait();
	}

	latest_news = std::async(std::launch::async, []() noexcept {
		auto html = augs::http_get_request("http://hypersomnia.pl/latest_post/");
		const auto delimiter = std::string("newsbegin");

		const auto it = html.find(delimiter);

		if (it != std::string::npos) {
			html = html.substr(it + delimiter.length());

			str_ops(html)
				.multi_replace_all({ "\r", "\n" }, "")
			;

			if (html.size() > 0) {
				return to_wstring(html);
			}
		}

		return std::wstring(L"Couldn't download and/or parse the latest news.");
	});
}

main_menu_setup::main_menu_setup(
	sol::state& lua,
	const main_menu_settings settings
) : menu_theme(settings.menu_theme_path) {
	query_latest_news(settings.latest_news_url);

	if (settings.skip_credits) {
		gui.show = true;
	}

	const auto menu_config_patch_path = "content/menu/config.lua";

	try {
		auto pfr = lua.do_string(augs::get_file_contents(menu_config_patch_path));
		
		if (pfr.valid()) {
			menu_config_patch = pfr;
		}
		else {
			LOG("Warning: problem loading %x: \n%x.", menu_config_patch_path, pfr.operator std::string());
		}
	}
	catch (const augs::ifstream_error& err) {
		LOG("Error loading file %x:\n%x\nMenu will apply no patch to config.", menu_config_patch_path, err.what());
	}

	float gain_fade_multiplier = 0.f;

	if (augs::file_exists(settings.menu_theme_path)) {
		menu_theme_source.bind_buffer(menu_theme);
		menu_theme_source.set_direct_channels(true);
		menu_theme_source.seek_to(static_cast<float>(settings.start_menu_music_at_secs));
		menu_theme_source.set_gain(0.f);
		menu_theme_source.play();
	}

	// TODO: actually load a cosmos with its resources from a file/folder
	const bool is_intro_scene_available = settings.menu_intro_scene_workspace_path.string().size() > 0;

	if (is_intro_scene_available) {
#if BUILD_TEST_SCENES
		intro.make_test_scene(lua, false);
#endif
	}

	// director.load_recording_from_file(settings.menu_intro_scene_entropy_path);

	const bool is_recording_available = is_intro_scene_available && director.is_recording_available();
	initial_step_number = intro.world.get_total_steps_passed();

	for (auto& m : gui.root.buttons) {
		m.hover_highlight_maximum_distance = 10.f;
		m.hover_highlight_duration_ms = 300.f;
	}

	gui.root.buttons[main_menu_button_type::CONNECT_TO_OFFICIAL_UNIVERSE].set_appearing_caption(L"Login to official universe");
	gui.root.buttons[main_menu_button_type::BROWSE_UNOFFICIAL_UNIVERSES].set_appearing_caption(L"Browse unofficial universes");
	gui.root.buttons[main_menu_button_type::HOST_UNIVERSE].set_appearing_caption(L"Host universe");
	gui.root.buttons[main_menu_button_type::CONNECT_TO_UNIVERSE].set_appearing_caption(L"Connect to universe");
	gui.root.buttons[main_menu_button_type::LOCAL_UNIVERSE].set_appearing_caption(L"Local universe");
	gui.root.buttons[main_menu_button_type::EDITOR].set_appearing_caption(L"Editor");
	gui.root.buttons[main_menu_button_type::SETTINGS].set_appearing_caption(L"Settings");
	gui.root.buttons[main_menu_button_type::CREATORS].set_appearing_caption(L"Founders");
	gui.root.buttons[main_menu_button_type::QUIT].set_appearing_caption(L"Quit");

	if (is_recording_available) {
		while (intro.world.get_total_seconds_passed() < settings.rewind_intro_scene_by_secs) {
			const auto entropy = cosmic_entropy(director.get_entropy_for_step(intro.world.get_total_steps_passed() - initial_step_number), intro.world);

			intro.world.advance(
				{ entropy, intro.logicals },
				[](auto) {},
				[](auto) {},
				[](auto) {}
			);
		}
	}
}

void main_menu_setup::draw_overlays(
	const augs::drawer_with_default output,
	const necessary_images_in_atlas& necessarys,
	const augs::baked_font& gui_font,
	const vec2i screen_size
) const {
	const auto game_logo = necessarys.at(assets::necessary_image_id::MENU_GAME_LOGO);
	const auto game_logo_size = game_logo.get_size();

	ltrb game_logo_rect;
	game_logo_rect.set_position({ screen_size.x / 2.f - game_logo_size.x / 2.f, 50.f });
	game_logo_rect.set_size(game_logo_size);

	output.aabb(game_logo, game_logo_rect);

	if (is_ready(latest_news)) {
		print_stroked(
			output,
			latest_news_pos,
			from_bbcode ( latest_news.get(), { gui_font, cyan } )
		);
	};
}