#pragma once
#include "game/detail/perks/perk_timing.h"

struct perk_appearance {
	// GEN INTROSPECTOR struct spell_appearance
	assets::game_image_id icon = assets::game_image_id::INVALID;
	std::wstring description;
	rgba bar_color;
	// END GEN INTROSPECTOR
	
	auto get_icon() const {
		return icon;
	}

	auto get_bar_color() const {
		return bar_color;
	}

	const auto& get_description() const {
		return description;
	}
};