#include "application/gui/start_server_gui.h"

#include "augs/misc/imgui/imgui_control_wrappers.h"
#include "augs/misc/imgui/imgui_utils.h"

#define SCOPE_CFG_NVP(x) format_field_name(std::string(#x)) + "##" + std::to_string(field_id++), scope_cfg.x

bool start_server_gui_state::perform(
	server_start_input& into
) {
	if (!show) {
		return false;
	}

	using namespace augs::imgui;

	bool result = false;

	centered_size_mult = 0.3f;

	auto window = make_scoped_window();

	if (!window) {
		return false;
	}

	{
		auto child = scoped_child("host view", ImVec2(0, -(ImGui::GetFrameHeightWithSpacing() + 4)));
		auto width = scoped_item_width(ImGui::GetWindowWidth() * 0.35f);

		// auto& scope_cfg = into;

		(void)into;
	}

	{
		auto scope = scoped_child("launch cancel");

		ImGui::Separator();

		if (ImGui::Button("Launch!")) {
			result = true;
			//show = false;
		}

		ImGui::SameLine();

		if (ImGui::Button("Cancel")) {
			show = false;
		}
	}

	return result;
}

