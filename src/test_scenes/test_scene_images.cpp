#include "augs/filesystem/file.h"
#include "augs/misc/lua_readwrite.h"

#include "view/viewables/game_image.h"
#include "view/viewables/regeneration/game_image_loadables.h"

#include "test_scenes/test_scenes_content.h"

#include "generated/introspectors.h"

void load_test_scene_images(
	sol::state& lua,
	game_image_loadables_map& all_loadables,
	game_image_metas_map& all_metas
) {
	using id_type = assets::game_image_id;

	/* 
		This additional reference is only to mitigate MSVC bug 
		whereby there is some(?) problem capturing "this" contents in lambdas.
	*/

	const auto directory = augs::path_type("content/official/gfx/");

	augs::for_each_enum_except_bounds([&](const id_type id) {
		if (found_in(all_loadables, id) || found_in(all_metas, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(id));

		game_image_loadables loadables;
		game_image_meta meta;

		loadables.source_image_path = augs::path_type(directory) += stem + ".png";

		try {
			if (
				const auto extra_loadables_path = augs::path_type(directory) += stem + ".extras.lua";
				augs::file_exists(extra_loadables_path)
			) {
				augs::load_from_lua_table(lua, loadables.extras, extra_loadables_path);
			}

			if (
				const auto meta_path = augs::path_type(directory) += stem + ".meta.lua";
				augs::file_exists(meta_path)
			) {
				augs::load_from_lua_table(lua, meta, meta_path);
			}
		}
		catch (augs::lua_deserialization_error err) {
			throw test_scene_asset_loading_error(
				"Error while loading additional properties for %x: %x",
				stem,
				err.what()
			);
		}

		all_loadables[id] = loadables;
		all_metas[id] = meta;
	});
}