#pragma once
#include "game/detail/render_layer_filter.h"

#include "application/setups/editor/gui/standard_window_mixin.h"
#include "application/setups/editor/property_editor/property_editor_settings.h"

struct editor_filters_gui : standard_window_mixin<editor_filters_gui> {
	using base = standard_window_mixin<editor_filters_gui>;
	using base::base;
	using introspect_base = base;

	void perform(
		const property_editor_settings& settings,
		augs::maybe<render_layer_filter>& viewing,
		augs::maybe<render_layer_filter>& selecting
	);
};
