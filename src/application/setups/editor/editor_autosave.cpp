#include "augs/filesystem/directory.h"

#include "application/intercosm.h"

#include "application/setups/editor/editor_popup.h"
#include "application/setups/editor/editor_autosave.h"
#include "application/setups/editor/editor_paths.h"
#include "application/setups/editor/editor_significant.h"

#include "augs/readwrite/lua_file.h"

void editor_autosave::save(
	sol::state& lua,
	const editor_significant& signi
) const {
	{
		editor_last_folders last_folders;
		last_folders.current_index = signi.current_index;
		last_folders.paths.reserve(signi.folders.size());

		for (const auto& f : signi.folders) {
			last_folders.paths.push_back(f.current_path);
		}

		augs::save_as_lua_table(lua, last_folders, get_last_folders_path());
	}

	for (const auto& f : signi.folders) {
		if (const auto& h = f.history;
			h.at_unsaved_revision() || h.was_modified()
		) {
			/* A write is required */
			if (f.is_untitled()) {
				/* The work is untitled anyway, so we save it in place. */ 
				f.save_folder();
			}
			else {
				auto autosave_path = f.get_autosave_path();
				augs::create_directories(autosave_path += "/");
				f.save_folder(autosave_path, ::get_project_name(f.current_path));
			}
		}
	}
}

void open_last_folders(
	sol::state& lua,
	editor_significant& signi
) {
	ensure(signi.folders.empty());

	std::vector<editor_popup> failures;

	try {
		const auto opened_folders = augs::load_from_lua_table<editor_last_folders>(lua, get_last_folders_path());

		for (const auto& real_path : opened_folders.paths) {
			try {
				try {
					auto new_folder = editor_folder(real_path);
					const auto autosave_path = new_folder.get_autosave_path();
					new_folder.load_folder(autosave_path, ::get_project_name(real_path));
					signi.folders.emplace_back(std::move(new_folder));

					if (!augs::exists(real_path)) {
						const auto display_autosave = augs::to_display_path(autosave_path);
						const auto display_real = augs::to_display_path(real_path);

						const auto message = typesafe_sprintf(
							"Found the autosave file %x,\nbut there is no %x!\nSave the file immediately!",
							display_autosave,
							display_real
						);

						failures.push_back({"Warning", message, ""});
					}
				}
				catch (editor_popup p) {
					auto new_folder = editor_folder(real_path);
					new_folder.load_folder();
					new_folder.history.mark_as_just_saved();
					signi.folders.emplace_back(std::move(new_folder));
				}
			}
			catch (editor_popup p) {
				failures.push_back(p);
			}
		}

		if (signi.folders.empty()) {
			signi.current_index = static_cast<folder_index>(-1);
		}
		else {
			/* The folder that was originally specified as current could have failed to load */

			signi.current_index = std::min(
				static_cast<folder_index>(signi.folders.size()) - 1,
				opened_folders.current_index
			);
		}
	}
	catch (...) {

	}

	if (!failures.empty()) {
		throw editor_popup::sum_all(failures);
	}
}

void editor_autosave::advance(
	sol::state& lua,
	const editor_significant& signi,
	const editor_autosave_settings& settings
) {
	if (last_settings != settings) {
		autosave_timer = {};
	}

	last_settings = settings;

	if (settings.enabled 
		&& settings.once_every_min <= autosave_timer.get<std::chrono::minutes>()
	) {
		save(lua, signi);
		autosave_timer.reset();
	}
}
