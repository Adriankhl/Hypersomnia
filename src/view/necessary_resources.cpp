#include "augs/templates/container_templates.h"
#include "augs/templates/introspect.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/gui/button_corners.h"

#include "view/game_drawing_settings.h"

#include "view/necessary_resources.h"

#include "view/viewables/regeneration/images_from_commands.h"
#include "view/viewables/regeneration/procedural_image_definition.h"

#include "augs/readwrite/lua_file.h"

necessary_fbos::necessary_fbos(
	const vec2i screen_size,
	const game_drawing_settings settings
) {
	apply(screen_size, settings);
}

void necessary_fbos::apply(
	const vec2i screen_size,
	const game_drawing_settings settings
) {
	if (const bool just_minimized = screen_size.is_zero()) {
		return;
	}

	auto reset = [screen_size](auto& fbo) {
		if (!fbo || fbo->get_size() != screen_size) {
			fbo.emplace(screen_size);
		}
	};

	reset(illuminating_smoke);
	reset(smoke);
	reset(light);
}

necessary_shaders::necessary_shaders(
	const augs::path_type& canon_directory,
	const augs::path_type& local_directory,
	const game_drawing_settings settings
) {
	augs::introspect(
		[&](const std::string label, auto& shader) {
			const auto canon_vsh_path = typesafe_sprintf("%x/%x.vsh", canon_directory, label);
			const auto local_vsh_path = typesafe_sprintf("%x/%x.vsh", local_directory, label);

			const auto final_vsh_path = augs::switch_path(
				canon_vsh_path,
				local_vsh_path
			);

			const auto final_fsh_path = augs::path_type(final_vsh_path).replace_extension(".fsh");
			
			try {
				shader.emplace(
					final_vsh_path,
					final_fsh_path
				);
			}
			catch (augs::graphics::shader_error err) {
				LOG(
					"There was a problem building %x.\n That shader was not set.",
					label
				);

				LOG(err.what());
			}
		},
		*this
	);

	if (illuminated) {
		illuminated->set_as_current();
		illuminated->set_uniform("basic_texture", 0);
		illuminated->set_uniform("light_texture", 2);
	}

	if (standard) {
		standard->set_as_current();
		standard->set_uniform("basic_texture", 0);
	}

	if (pure_color_highlight) {
		pure_color_highlight->set_as_current();
		pure_color_highlight->set_uniform("basic_texture", 0);
	}

	if (circular_bars) {
		circular_bars->set_as_current();
		circular_bars->set_uniform("basic_texture", 0);
	}

	if (smoke) {
		smoke->set_as_current();
		smoke->set_uniform("smoke_texture", 1);
		smoke->set_uniform("light_texture", 2);
	}

	if (illuminating_smoke) {
		illuminating_smoke->set_as_current();
		illuminating_smoke->set_uniform("smoke_texture", 3);
	}

	if (specular_highlights) {
		specular_highlights->set_as_current();
		specular_highlights->set_uniform("basic_texture", 0);
		specular_highlights->set_uniform("light_texture", 2);
	}
}

necessary_sound_buffers::necessary_sound_buffers(
	const augs::path_type& directory
) try :
	button_click(augs::sound_data(typesafe_sprintf("%x/button_click.wav", directory))),
	button_hover(augs::sound_data(typesafe_sprintf("%x/button_hover.wav", directory)))
{}
catch (const augs::sound_decoding_error err) {
	throw necessary_resource_loading_error(err.what());
}

augs::path_type get_procedural_image_path(const augs::path_type& from_source_path) {
	return typesafe_sprintf(GENERATED_FILES_DIR "%x", from_source_path);
}

necessary_image_loadables_map::necessary_image_loadables_map(
	sol::state& lua,
	const augs::path_type& directory,
	const bool force_regenerate
) {
	using id_type = assets::necessary_image_id;

	/* 
		This additional reference is only to mitigate MSVC bug 
		whereby there is some(?) problem capturing "this" contents in lambdas.
	*/

	augs::for_each_enum_except_bounds([&](const id_type id) {
		if (found_in(*this, id)) {
			return;
		}

		const auto stem = to_lowercase(augs::enum_to_string(id));

		game_image_loadables definition_template;

		if (
			const auto additional_properties_path = typesafe_sprintf("%x/%x.lua", directory, stem);
			augs::file_exists(additional_properties_path)
		) {
			try {
				augs::load_from_lua_table(
					lua,
					definition_template,
					additional_properties_path
				);
			}
			catch (augs::lua_deserialization_error err) {
				throw necessary_resource_loading_error(
					"Failed to load additional properties for %x (%x).\nNot a valid lua table.\n%x",
					stem, 
					additional_properties_path,
					err.what()
				);
			}
			catch (augs::ifstream_error err) {
				throw necessary_resource_loading_error(
					"Failed to load additional properties for %x (%x).\nFile might be corrupt.\n%x",
					stem,
					additional_properties_path,
					err.what()
				);
			}
		}

		if (
			const auto source_image_path = typesafe_sprintf("%x/%x.png", directory, stem);
			augs::file_exists(source_image_path)
		) {
			definition_template.source_image_path = source_image_path;
			emplace(id, definition_template);
		}
		else if (
			const auto procedural_definition_path = typesafe_sprintf("%x/procedural/%x.lua", directory, stem);
			augs::file_exists(procedural_definition_path)
		) {
			procedural_image_definition def;

			try {
				augs::load_from_lua_table(lua, def, procedural_definition_path);
			}
			catch (augs::lua_deserialization_error err) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x).\nNot a valid lua table.\n%x",
					stem,
					procedural_definition_path,
					err.what()
				);
			}
			catch (augs::ifstream_error err) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x).\nFile might be corrupt.\n%x",
					stem,
					procedural_definition_path,
					err.what()
				);
			}
			
			if (
				const bool exactly_one = def.button_with_corners.has_value() != def.image_from_commands.has_value();
				!exactly_one
			) {
				throw necessary_resource_loading_error(
					"Failed to load procedural image definition for %x (%x):\n%x",
					stem,
					procedural_definition_path,
					"Either none or more than one type of procedural image have been specified."
				);
			}

			if (def.button_with_corners) {
				const auto path_template = get_procedural_image_path(
					typesafe_sprintf("%x/procedural/%x_%x.png", directory, stem)
				);
				
				const auto input = def.button_with_corners.value();
		
				regenerate_button_with_corners(path_template, input, force_regenerate);
		
				const auto first = id;

				augs::for_each_enum_except_bounds([&](const button_corner_type type) {
					definition_template.source_image_path = typesafe_sprintf(path_template.string(), get_filename_for(type));
					
					emplace(
						static_cast<id_type>(
							static_cast<int>(first) + static_cast<int>(type)
						), 
						definition_template
					);
				});
			}
			else if (def.image_from_commands) {
				const auto generated_image_path = get_procedural_image_path(
					typesafe_sprintf("%x/procedural/%x.png", directory, stem)
				);
		
				regenerate_image_from_commands(
					generated_image_path,
					def.image_from_commands.value(),
					force_regenerate
				);
		
				definition_template.source_image_path = generated_image_path;
				emplace(id, definition_template);
			}
		}

		if (const bool nothing_loaded = definition_template.source_image_path.empty()) {
			throw necessary_resource_loading_error(
				"Failed to load necessary image: %x.\n%x",
				stem,
				"No source image exists, nor does a procedural definition."
			);
		}
	});
}
