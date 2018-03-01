#include "intercosm.h"

#include "game/organization/all_messages_includes.h"
#include "game/organization/all_component_includes.h"

#include "test_scenes/test_scenes_content.h"

#include "test_scenes/scenes/testbed.h"
#include "test_scenes/scenes/minimal_scene.h"
#include "test_scenes/test_scene_settings.h"

#include "augs/readwrite/lua_file.h"
#include "augs/readwrite/byte_file.h"

#if BUILD_TEST_SCENES
void intercosm::make_test_scene(
	sol::state& lua, 
	const test_scene_settings settings
) {
	world.clear();
	viewables = {};

#if !STATICALLY_ALLOCATE_ENTITIES
	cosmic::reserve_storage_for_entities(world, 3000u);
#endif

	const auto caches = populate_test_scene_images_and_sounds(lua, viewables);
	populate_test_scene_viewables(lua, caches, viewables);

	auto reloader = [&](auto populator){
		world.change_common_significant([&](cosmos_common_significant& common){
			auto& logicals = common.logical_assets;
			logicals = {};
			populate_test_scene_logical_assets(logicals);

			populator.populate(caches, common);

			return changer_callback_result::DONT_REFRESH;
		});

		cosmic::change_solvable_significant(world, [settings](auto& s){
			s.clock.delta = augs::delta::steps_per_second(settings.scene_tickrate); 
			return changer_callback_result::REFRESH;
		});

		locally_viewed = populator.populate_with_entities(caches, make_logic_step_input({}));
	};

	if (settings.create_minimal) {
		reloader(test_scenes::minimal_scene());
	}
	else {
		reloader(test_scenes::testbed());
	}
}
#endif

void intercosm::save(const intercosm_path_op op) const {
	const auto effective_extension = op.path.extension();

	if (effective_extension == ".int") {
		save_as_int(op.path);
	}
	else if (effective_extension == ".lua") {
		save_as_lua(op);
	}
}

void intercosm::load(const intercosm_path_op op) {
	const auto effective_extension = op.path.extension();

	if (effective_extension == ".int") {
		load_as_int(op.path);
	}
	else if (effective_extension == ".lua") {
		load_as_lua(op);
	}
}

void intercosm::save_as_lua(const intercosm_path_op op) const {
	augs::save_as_lua_table(op.lua, *this, op.path);
}

void intercosm::load_as_lua(const intercosm_path_op op) {
	const auto display_path = augs::to_display_path(op.path);

	try {
		augs::recursive_clear(version);
		version.commit_number = 0;

		augs::load_from_lua_table(op.lua, *this, op.path);

		/* TODO: Check version integrity */

		version = hypersomnia_version();
	}
	catch (const cosmos_loading_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::stream_read_error err) {
		throw intercosm_loading_error{
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::lua_deserialization_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nNot a valid lua table.", display_path),
			err.what()
		};
	}
	catch (const augs::ifstream_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be missing.", display_path),
			err.what()
		};
	}
}

void intercosm::save_as_int(const augs::path_type& path) const {
	augs::save_as_bytes(*this, path);
}

void intercosm::load_as_int(const augs::path_type& path) {
	const auto display_path = augs::to_display_path(path);

	try {
		augs::recursive_clear(version);
		version.commit_number = 0;

		augs::load_from_bytes(*this, path);

		/* TODO: Check version integrity */

		version = hypersomnia_version();
	}
	catch (const cosmos_loading_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::stream_read_error err) {
		throw intercosm_loading_error{
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be corrupt.", display_path),
			err.what()
		};
	}
	catch (const augs::ifstream_error err) {
		throw intercosm_loading_error {
			"Error",
			typesafe_sprintf("Failed to load %x.\nFile might be missing.", display_path),
			err.what()
		};
	}
}