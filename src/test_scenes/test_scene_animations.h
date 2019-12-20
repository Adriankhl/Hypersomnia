#pragma once
#include "test_scenes/test_id_to_pool_id.h"
#include "test_scenes/test_scene_images.h"

enum class test_scene_plain_animation_id {
	// GEN INTROSPECTOR enum class test_scene_plain_animation_id
	CAST_BLINK,
	WANDERING_PIXELS_ANIMATION,
	BAKA47_SHOT,
	DATUM_GUN_SHOT,
	BLUNAZ_SHOT,
	CALICO_SHOT,
	DEAGLE_SHOT,
	BULWARK_SHOT,

	METROPOLIS_TORSO_BARE_WALK,
	METROPOLIS_TORSO_BARE_SHOT,
	METROPOLIS_TORSO_KNIFE_WALK,

	METROPOLIS_TORSO_KNIFE_PRIM,
	METROPOLIS_TORSO_KNIFE_PRIM_RETURN,
	METROPOLIS_TORSO_KNIFE_SECD,
	METROPOLIS_TORSO_KNIFE_SECD_RETURN,

	METROPOLIS_TORSO_PISTOL_WALK,
	METROPOLIS_TORSO_PISTOL_SHOT,
	METROPOLIS_TORSO_PISTOL_PTM,
	METROPOLIS_TORSO_PISTOL_GTM,
	METROPOLIS_TORSO_RIFLE_WALK,
	METROPOLIS_TORSO_RIFLE_SHOT,
	METROPOLIS_TORSO_RIFLE_PTM,
	METROPOLIS_TORSO_RIFLE_GTM,
	METROPOLIS_TORSO_HEAVY_WALK,
	METROPOLIS_TORSO_HEAVY_SHOT,
	METROPOLIS_TORSO_HEAVY_GTM,
	METROPOLIS_TORSO_AKIMBO_WALK,
	METROPOLIS_TORSO_AKIMBO_SHOT,

	METROPOLIS_TORSO_SNIPER_CHAMBER,

	RESISTANCE_TORSO_BARE_WALK,
	RESISTANCE_TORSO_BARE_SHOT,
	RESISTANCE_TORSO_KNIFE_WALK,

	RESISTANCE_TORSO_KNIFE_PRIM,
	RESISTANCE_TORSO_KNIFE_PRIM_RETURN,
	RESISTANCE_TORSO_KNIFE_SECD,
	RESISTANCE_TORSO_KNIFE_SECD_RETURN,

	RESISTANCE_TORSO_PISTOL_WALK,
	RESISTANCE_TORSO_PISTOL_SHOT,
	RESISTANCE_TORSO_PISTOL_PTM,
	RESISTANCE_TORSO_PISTOL_GTM,
	RESISTANCE_TORSO_RIFLE_WALK,
	RESISTANCE_TORSO_RIFLE_SHOT,
	RESISTANCE_TORSO_RIFLE_PTM,
	RESISTANCE_TORSO_RIFLE_GTM,
	RESISTANCE_TORSO_HEAVY_WALK,
	RESISTANCE_TORSO_HEAVY_SHOT,
	RESISTANCE_TORSO_HEAVY_GTM,
	RESISTANCE_TORSO_AKIMBO_WALK,
	RESISTANCE_TORSO_AKIMBO_SHOT,

	RESISTANCE_TORSO_SNIPER_CHAMBER,

	SILVER_TROUSERS,
	SILVER_TROUSERS_STRAFE,

	YELLOW_FISH,
	DARKBLUE_FISH,
	CYANVIOLET_FISH,
	JELLYFISH,
	DRAGON_FISH,

	FLOWER_PINK,
	FLOWER_CYAN,

	CONSOLE_LIGHT,

	WATER_SURFACE,

	SMALL_BUBBLE_LB,
	SMALL_BUBBLE_LT,
	SMALL_BUBBLE_RB,
	SMALL_BUBBLE_RT,

	MEDIUM_BUBBLE,
	BIG_BUBBLE,

	PINK_CORAL,

	BOMB,
	BOMB_ARMED,

	ZAMIEC_SHOT,

	ASSAULT_RATTLE,

	COUNT
	// END GEN INTROSPECTOR
};

using test_scene_torso_animation_id = test_scene_plain_animation_id;
using test_scene_legs_animation_id = test_scene_plain_animation_id;

inline auto to_animation_id(const test_scene_plain_animation_id id) {
	return to_pool_id<assets::plain_animation_id>(id);
}
