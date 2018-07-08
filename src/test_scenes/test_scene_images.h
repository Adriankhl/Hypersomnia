#pragma once
#include "test_scenes/test_id_to_pool_id.h"

enum class test_scene_image_id {
	// GEN INTROSPECTOR enum class test_scene_image_id
	BLANK = 0,

	BLANK_2X2,

	CRATE,

	TRUCK_INSIDE,
	TRUCK_FRONT,

	JMIX114,

	TEST_CROSSHAIR,

	SMOKE_1,
	SMOKE_2,
	SMOKE_3,
	SMOKE_4,
	SMOKE_5,
	SMOKE_6,

	PIXEL_THUNDER_1,
	PIXEL_THUNDER_2,
	PIXEL_THUNDER_3,
	PIXEL_THUNDER_4,
	PIXEL_THUNDER_5,

	ASSAULT_RIFLE,
	BILMER2000,
	KEK9,
	URBAN_CYAN_MACHETE,

	LEWSII,
	LEWSII_MAG,

	FLOWER_PINK_1,
	FLOWER_CYAN_1,
	CONSOLE_LIGHT_1,
	VINDICATOR_SHOOT_1,
	DATUM_GUN_SHOOT_1,

	SOIL,
	FLOOR,

	SAMPLE_MAGAZINE,
	SMALL_MAGAZINE,
	ROUND_TRACE,
	ELECTRIC_MISSILE,
	PINK_CHARGE,
	PINK_SHELL,
	CYAN_CHARGE,
	CYAN_SHELL,
	RED_CHARGE,
	RED_SHELL,
	GREEN_CHARGE,
	GREEN_SHELL,
	STEEL_CHARGE,
	STEEL_SHELL,
	STEEL_ROUND,

	STEEL_ROUND_REMNANT_1,
	STEEL_ROUND_REMNANT_2,
	STEEL_ROUND_REMNANT_3,

	BACKPACK,
	BROWN_BACKPACK,

	HAVE_A_PLEASANT,
	AWAKENING,
	METROPOLIS,

	BRICK_WALL,
	ROAD,
	ROAD_FRONT_DIRT,

	CAST_BLINK_1,

	SILVER_TROUSERS_1,
	SILVER_TROUSERS_STRAFE_1,

	METROPOLIS_CHARACTER_BARE_1,
	METROPOLIS_CHARACTER_RIFLE_1,
	METROPOLIS_CHARACTER_AKIMBO_1,

	METROPOLIS_CHARACTER_HEAVY_1,
	METROPOLIS_CHARACTER_HEAVY_SHOOT_1,

	RESISTANCE_CHARACTER_BARE_1,
	RESISTANCE_CHARACTER_RIFLE_1,
	RESISTANCE_CHARACTER_RIFLE_SHOOT_1,

	BIG_BUBBLE_1,
	MEDIUM_BUBBLE_1,
	SMALL_BUBBLE_LB_1,
	SMALL_BUBBLE_LT_1,
	SMALL_BUBBLE_RB_1,
	SMALL_BUBBLE_RT_1,

	YELLOW_FISH_1,
	DARKBLUE_FISH_1,
	CYANVIOLET_FISH_1,
	JELLYFISH_1,
	DRAGON_FISH_1,

	AQUARIUM_BOTTOM_LAMP_LIGHT,
	AQUARIUM_BOTTOM_LAMP_BODY,

	AQUARIUM_HALOGEN_1_LIGHT,
	AQUARIUM_HALOGEN_1_BODY,

	AQUARIUM_SAND_EDGE,
	AQUARIUM_SAND_CORNER,

	AQUARIUM_SAND_1,
	AQUARIUM_SAND_2,

	AQUARIUM_GLASS_START,
	AQUARIUM_GLASS,

	LAB_WALL_SMOOTH_END,
	LAB_WALL,
	LAB_WALL_A2,

	LAB_WALL_CORNER_CUT,
	LAB_WALL_CORNER_SQUARE,

	WATER_SURFACE_1,
	PINK_CORAL_1,

	DUNE_BIG,
	DUNE_SMALL,

	METROPOLIS_HEAD,
	RESISTANCE_HEAD,

	TRUCK_ENGINE,

	HEALTH_ICON,
	PERSONAL_ELECTRICITY_ICON,
	CONSCIOUSNESS_ICON,

	AMPLIFIER_ARM,

	SPELL_HASTE_ICON,
	SPELL_ELECTRIC_SHIELD_ICON,
	SPELL_ELECTRIC_TRIAD_ICON,
	SPELL_FURY_OF_THE_AEONS_ICON,
	SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON,
	SPELL_EXALTATION_ICON,
	SPELL_ECHOES_OF_THE_HIGHER_REALMS_ICON,

	PERK_HASTE_ICON,
	PERK_ELECTRIC_SHIELD_ICON,

	FORCE_GRENADE,
	PED_GRENADE,
	INTERFERENCE_GRENADE,

	FORCE_GRENADE_RELEASED,
	PED_GRENADE_RELEASED,
	INTERFERENCE_GRENADE_RELEASED,
	FORCE_ROCKET,

	FAN,

	COUNT
	// END GEN INTROSPECTOR
};

inline auto to_image_id(const test_scene_image_id id) {
	return to_pool_id<assets::image_id>(id);
}
