#include "augs/misc/simple_pair.h"
#include "augs/templates/for_each_std_get.h"

#include "augs/readwrite/memory_stream.h"

#include "augs/misc/imgui/imgui_utils.h"
#include "augs/misc/imgui/imgui_scope_wrappers.h"
#include "augs/misc/imgui/imgui_control_wrappers.h"

#include "application/intercosm.h"
#include "application/setups/editor/editor_folder.h"
#include "application/setups/editor/gui/editor_common_state_gui.h"

#include "application/setups/editor/property_editor/property_editor_gui.h"
#include "application/setups/editor/property_editor/property_editor_structs.h"

#include "augs/readwrite/memory_stream.h"
#include "augs/readwrite/byte_readwrite.h"

template <class T>
struct should_skip_in_common : std::bool_constant<
	is_one_of_v<T, all_logical_assets, all_entity_flavours>
> {};

static void edit_common(
	property_editor_gui& state,
	const cosmos_common_significant& signi,
	const editor_command_input in
) {
	using namespace augs::imgui;

	auto make_property_id = [&](const field_address field) {
		return field;
	};

	const auto property_location = [&]() {
		return typesafe_sprintf(" (Common state)");
	}();

	/* Linker error fix */
	auto& history = in.folder.history;

	auto post_new_change = [&](
		const auto& description,
		const field_address field,
		const auto& new_content
	) {
		change_common_state_command cmd;
		cmd.field = field;

		cmd.value_after_change = augs::to_bytes(new_content);
		cmd.built_description = description + property_location;

		history.execute_new(cmd, in);
	};

	auto rewrite_last_change = [&](
		const auto& description,
		const auto& new_content
	) {
		auto& last = history.last_command();

		if (auto* const cmd = std::get_if<change_common_state_command>(std::addressof(last))) {
			cmd->built_description = description + property_location;
			cmd->rewrite_change(augs::to_bytes(new_content), in);
		}
		else {
			LOG("WARNING! There was some problem with tracking activity of editor controls.");
		}
	};

	general_edit_properties<should_skip_in_common>(
		state, 
		signi,
		make_property_id,
		post_new_change,
		rewrite_last_change
	);
}

void editor_common_state_gui::open() {
	show = true;
	acquire_once = true;
	ImGui::SetWindowFocus("Common state");
}

void editor_common_state_gui::perform(const editor_command_input in) {
	if (!show) {
		return;
	}

	using namespace augs::imgui;

	auto entities = scoped_window("Common state", &show);
	auto& work = *in.folder.work;
	auto& cosm = work.world;

	if (acquire_once) {
		ImGui::SetKeyboardFocusHere();
		acquire_once = false;
	}

	ImGui::Columns(2); // 4-ways, with border
	next_column_text_disabled("Details");
	ImGui::Separator();

	edit_common(
		properties_gui,
		cosm.get_common_significant(),
		in
	);
}
