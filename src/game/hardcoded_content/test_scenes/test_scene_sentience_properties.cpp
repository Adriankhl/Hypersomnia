#include "game/hardcoded_content/all_hardcoded_content.h"
#include "game/detail/spells/spell_structs.h"
#include "game/transcendental/cosmos.h"

#include "game/detail/spells/all_spells.h"
#include "game/detail/perks/all_perks.h"

void load_test_scene_sentience_properties(
	cosmos_global_state& state
) {
	auto& perks = state.perks;
	auto& spells = state.spells;
	auto& meters = state.meters;

	const rgba turqoise_spell_color = { 0, 225, 255 };
	const rgba blue_spell_border = cyan; //{ 0, 128, 209, 255 };
	const rgba green_spell_color = { 0, 255, 0, 255 };

	{
		auto& d = std::get<haste>(spells);
		d.common.cooldown_ms = 5000;
		d.common.personal_electricity_required = 60;
		d.common.associated_color = green_spell_color;
		d.perk_duration_seconds = 33;

		d.appearance.incantation = L"treximo";

		d.appearance.name = L"[color=green]Haste[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Increases movement speed for %x seconds.[/color]", 
			d.perk_duration_seconds
		);

		d.appearance.icon = assets::game_image_id::SPELL_HASTE_ICON;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;
	}

	{
		auto& d = std::get<exaltation>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 50;
		d.common.associated_color = green;

		d.appearance.incantation = L"efforia";

		d.appearance.name = L"[color=green]Exaltation[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Stabilizes functions of the physical body.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_EXALTATION_ICON;
		d.basic_healing_amount = 34;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;
		d.common.cast_sparkles.modifier.scale_amounts = 1.3f;
		d.common.cast_sparkles.modifier.scale_lifetimes = 1.3f;
	}

	{
		auto& d = std::get<echoes_of_the_higher_realms>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 90;
		d.common.associated_color = yellow;

		d.appearance.incantation = L"armonia";

		d.appearance.name = L"[color=yellow]Echoes of the higher realms[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Restores resonance of mind with the body.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_ECHOES_OF_THE_HIGHER_REALMS_ICON;
		d.basic_healing_amount = 132;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;
		d.common.cast_sparkles.modifier.scale_amounts = 1.3f;
		d.common.cast_sparkles.modifier.scale_lifetimes = 1.3f;
	}

	{
		auto& d = std::get<fury_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 100;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"mania aiones";

		d.appearance.name = L"[color=cyan]Fury of the Aeons[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes instant damage around the caster.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_FURY_OF_THE_AEONS_ICON;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;

		{
			auto& in = d.explosion;

			in.effective_radius = 250.f;
			in.damage = 88.f;
			in.impact_force = 150.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.sound_effect = assets::sound_buffer_id::EXPLOSION;
			in.sound_gain = 1.2f;
			in.type = adverse_element_type::FORCE;
		}
	}

	{
		auto& d = std::get<electric_triad>(spells);
		d.common.cooldown_ms = 3000;
		d.common.personal_electricity_required = 120;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"energeia triada";

		d.appearance.name = L"[color=cyan]Electric Triad[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Spawns three electric missiles\nhoming towards hostile entities.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_ELECTRIC_TRIAD_ICON;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;
	}

	{
		auto& d = std::get<ultimate_wrath_of_the_aeons>(spells);
		d.common.cooldown_ms = 2000;
		d.common.personal_electricity_required = 260;
		d.common.associated_color = blue_spell_border;

		d.appearance.incantation = L"megalyteri aiones via";

		d.appearance.name = L"[color=cyan]Ultimate Wrath of the Aeons[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]Causes massive damage around the caster.\nRequires delay to initiate.[/color]"
		);

		d.appearance.icon = assets::game_image_id::SPELL_ULTIMATE_WRATH_OF_THE_AEONS_ICON;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;

		d.charging_particles.id = assets::particle_effect_id::CAST_CHARGING;
		d.charging_particles.modifier.scale_lifetimes = 1.3f;
		d.charging_sound.id = assets::sound_buffer_id::CAST_CHARGING;

		{
			standard_explosion_input in;
			in.damage = 88.f;
			in.inner_ring_color = cyan;
			in.outer_ring_color = white;
			in.type = adverse_element_type::FORCE;

			{
				in.effective_radius = 200.f;
				in.impact_force = 150.f;
				in.sound_gain = 1.2f;
				in.sound_effect = assets::sound_buffer_id::EXPLOSION;

				d.explosions[0] = in;
			}
			
			{
				in.effective_radius = 400.f;
				in.impact_force = 200.f;
				in.sound_gain = 1.0f;
				in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

				d.explosions[1] = in;
			}
			
			{
				in.effective_radius = 600.f;
				in.impact_force = 250.f;
				in.sound_gain = 1.2f;
				in.sound_effect = assets::sound_buffer_id::GREAT_EXPLOSION;

				d.explosions[2] = in;
			}
		}
	}

	{
		auto& d = std::get<electric_shield>(spells);
		d.common.cooldown_ms = 5000;
		d.perk_duration_seconds = 60;
		d.common.personal_electricity_required = 50;
		d.common.associated_color = turqoise_spell_color;

		d.appearance.incantation = L"energeia aspida";

		d.appearance.name = L"[color=turquoise]Electric Shield[/color]";
		d.appearance.description = typesafe_sprintf(
			L"[color=vsdarkgray]For %x seconds, damage is absorbed\nby [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]",
			d.perk_duration_seconds
		);

		d.appearance.icon = assets::game_image_id::SPELL_ELECTRIC_SHIELD_ICON;

		d.common.cast_successful_sound.id = assets::sound_buffer_id::CAST_SUCCESSFUL;
		d.common.cast_sparkles.id = assets::particle_effect_id::CAST_SPARKLES;
		d.common.cast_sparkles.modifier.colorize = d.common.associated_color;
	}

	{
		auto& p = std::get<electric_shield_perk>(perks);
		p.appearance.description = L"[color=turquoise]Electric shield[/color]\n[color=vsdarkgray]Damage is absorbed by [/color][color=cyan]Personal Electricity[/color][color=vsdarkgray] instead of [/color][color=red]Health[/color][color=vsdarkgray].[/color]";
		p.appearance.icon = assets::game_image_id::PERK_ELECTRIC_SHIELD_ICON;
		p.appearance.bar_color = turquoise - rgba(30, 30, 30, 0);
	}

	{
		auto& p = std::get<haste_perk>(perks);
		p.appearance.description = L"[color=green]Haste[/color]\n[color=vsdarkgray]You move faster.[/color]";
		p.appearance.icon = assets::game_image_id::PERK_HASTE_ICON;
		p.appearance.bar_color = green - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<health_meter>(meters);

		m.appearance.description = L"[color=red]Health points:[/color] %x/%x\n[color=vsdarkgray]Stability of the physical body.[/color]";
		m.appearance.icon = assets::game_image_id::HEALTH_ICON;
		m.appearance.bar_color = red - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<personal_electricity_meter>(meters);

		m.appearance.description = L"[color=cyan]Personal electricity:[/color] %x/%x\n[color=vsdarkgray]Mind-programmable matter.[/color]";
		m.appearance.icon = assets::game_image_id::PERSONAL_ELECTRICITY_ICON;
		m.appearance.bar_color = cyan - rgba(30, 30, 30, 0);
	}

	{
		auto& m = std::get<consciousness_meter>(meters);

		m.appearance.description = L"[color=orange]Consciousness:[/color] %x/%x\n[color=vsdarkgray]Attunement of soul with the body.[/color]";
		m.appearance.icon = assets::game_image_id::CONSCIOUSNESS_ICON;
		m.appearance.bar_color = orange - rgba(30, 30, 30, 0);
	}
}