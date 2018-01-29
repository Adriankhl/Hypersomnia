#include "minimal_scene.h"

#include "augs/templates/algorithm_templates.h"

#include "game/enums/party_category.h"

#include "test_scenes/test_scene_types.h"
#include "test_scenes/ingredients/ingredients.h"
#include "test_scenes/test_scenes_content.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"
#include "game/organization/all_messages_includes.h"
#include "game/transcendental/logic_step.h"

#include "game/detail/inventory/perform_transfer.h"

namespace test_scenes {
	void minimal_scene::populate(const loaded_game_image_caches& caches, cosmos_common_significant& common) const {
		populate_test_scene_types(caches, common.all_entity_types);
		load_test_scene_sentience_properties(common);

		auto& common_assets = common.assets;
		common_assets.cast_unsuccessful_sound.id = assets::sound_buffer_id::CAST_UNSUCCESSFUL;
		common_assets.ped_shield_impact_sound.id = assets::sound_buffer_id::EXPLOSION;
		common_assets.ped_shield_destruction_sound.id = assets::sound_buffer_id::GREAT_EXPLOSION;
		common_assets.exhausted_smoke_particles.id = assets::particle_effect_id::EXHAUSTED_SMOKE;
		common_assets.exploding_ring_smoke = assets::particle_effect_id::EXPLODING_RING_SMOKE;
		common_assets.exploding_ring_sparkles = assets::particle_effect_id::EXPLODING_RING_SPARKLES;
		common_assets.thunder_remnants = assets::particle_effect_id::THUNDER_REMNANTS;

		std::get<electric_triad>(common.spells).missile_flavour = to_entity_type_id(test_scene_type::ELECTRIC_MISSILE);
	}

	entity_id minimal_scene::populate(const loaded_game_image_caches& metas, const logic_step step) const {
		auto& world = step.get_cosmos();

		const int num_characters = 1;

		std::vector<entity_id> new_characters;
		new_characters.resize(num_characters);

		auto character = [&](const size_t i) {
			return i < new_characters.size() ? world[new_characters.at(i)] : world[entity_id()];
		};

		for (int i = 0; i < num_characters; ++i) {
			components::transform transform;

			if (i == 0) {
			}
			else if (i == 1) {
				transform.pos.x += 200;
			}

			const auto new_character = prefabs::create_sample_complete_character(step, transform, typesafe_sprintf("player%x", i), 1);

			new_characters[i] = new_character;

			if (i == 0) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::RESISTANCE_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::METROPOLIS_CITIZEN;
			}
			else if (i == 1) {
				new_character.get<components::sentience>().get<health_meter_instance>().set_value(100);
				new_character.get<components::sentience>().get<health_meter_instance>().set_maximum_value(100);
				new_character.get<components::attitude>().parties = party_category::METROPOLIS_CITIZEN;
				new_character.get<components::attitude>().hostile_parties = party_category::RESISTANCE_CITIZEN;
			}

			auto& sentience = new_character.get<components::sentience>();


			fill_range(sentience.learned_spells, true);
		}

		const auto rifle2 = prefabs::create_sample_rifle(step, vec2(100, -500 + 50),
			prefabs::create_sample_magazine(step, vec2(100, -650),
				prefabs::create_cyan_charge(step, vec2(0, 0), true ? 1000 : 5)));
		
		prefabs::create_force_grenade(step, { 100, 100 });
		prefabs::create_force_grenade(step, { 200, 100 });
		prefabs::create_force_grenade(step, { 300, 100});

		// _controlfp(0, _EM_OVERFLOW | _EM_ZERODIVIDE | _EM_INVALID | _EM_DENORMAL);
		return new_characters[0];
	}
}
