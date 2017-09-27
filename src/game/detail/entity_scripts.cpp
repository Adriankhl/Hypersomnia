#include "entity_scripts.h"
#include "game/components/movement_component.h"
#include "game/components/gun_component.h"
#include "game/components/melee_component.h"
#include "game/components/car_component.h"
#include "game/components/sentience_component.h"
#include "game/components/missile_component.h"
#include "game/components/attitude_component.h"
#include "game/components/container_component.h"
#include "game/components/sender_component.h"
#include "game/detail/inventory/inventory_utils.h"
#include "game/detail/inventory/inventory_slot.h"
#include "game/transcendental/entity_handle.h"
#include "game/transcendental/cosmos.h"
#include "augs/templates/container_templates.h"

#include <Box2D\Dynamics\b2WorldCallbacks.h>

void unset_input_flags_of_orphaned_entity(entity_handle e) {
	auto* const gun = e.find<components::gun>();
	auto* const melee = e.find<components::melee>();
	auto* const car = e.find<components::car>();
	auto* const movement = e.find<components::movement>();
	auto* const damage = e.find<components::missile>();

	if (car) {
		car->reset_movement_flags();
	}

	if (movement) {
		movement->reset_movement_flags();
	}

	if (gun) {
		gun->is_trigger_pressed = false;
	}

	if (melee) {
		melee->reset_weapon(e);
	}
}

bool isLeft(const vec2 a, const vec2 b, const vec2 c) {
	return ((b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x)) > 0;
}

identified_danger assess_danger(
	const const_entity_handle victim, 
	const const_entity_handle danger
) {
	identified_danger result;
	const auto& cosmos = victim.get_cosmos();

	const auto* const sentience = victim.find<components::sentience>();
	if (!sentience) return result;

	const auto& s = *sentience;

	result.danger = danger;

	const auto* const damage = danger.find<components::missile>();
	const auto* const attitude = danger.find<components::attitude>();

	if ((!damage && !attitude) || (damage && danger.get<components::sender>().is_sender_subject(victim))) {
		return result;
	}

	const auto victim_pos = victim.get_logic_transform().pos;
	const auto danger_pos = danger.get_logic_transform().pos;
	const auto danger_vel = danger.get_owner_body().get_effective_velocity();
	const auto danger_speed = danger_vel.length();
	const auto danger_dir = (danger_pos - victim_pos);
	const float danger_distance = danger_dir.length();

	if (danger_speed > 10) {
		result.recommended_evasion = isLeft(danger_pos, danger_pos + danger_vel, victim_pos) ? danger_vel.perpendicular_cw() : -danger_vel.perpendicular_cw();
	}
	else {
		result.recommended_evasion = -danger_dir;
	}

	result.recommended_evasion.normalize();

	//-danger_dir / danger_distance;

	const float comfort_zone_disturbance_ratio = (s.comfort_zone - danger_distance)/s.comfort_zone;

	if (comfort_zone_disturbance_ratio < 0) {
		return result;
	}

	if (damage) {
		result.amount += comfort_zone_disturbance_ratio * damage->damage_amount*4;
	}

	if (attitude) {
		const auto att = calculate_attitude(danger, victim);
		
		if (is_hostile(att)) {
			result.amount += comfort_zone_disturbance_ratio * sentience->danger_amount_from_hostile_attitude;
		}
	}
	
	return result;
}

attitude_type calculate_attitude(const const_entity_handle targeter, const const_entity_handle target) {
	const auto& targeter_attitude = targeter.get<components::attitude>();
	const auto* const target_attitude = target.find<components::attitude>();

	if (target_attitude) {
		if (targeter_attitude.hostile_parties & target_attitude->parties) {
			return attitude_type::WANTS_TO_KILL;
		}
		else if (targeter_attitude.parties & target_attitude->parties) {
			return attitude_type::WANTS_TO_HEAL;
		}
	}

	if(found_in(targeter_attitude.specific_hostile_entities, target)) {
		return attitude_type::WANTS_TO_KILL;
	}

	return attitude_type::NEUTRAL;
}


float assess_projectile_velocity_of_weapon(const const_entity_handle weapon) {
	if (weapon.dead()) {
		return 0.f;
	}

	// auto ch = weapon[slot_function::GUN_CHAMBER];
	// 
	// if (ch.has_items()) {
	// 	ch.get_items_inside()[0][child_entity_name::CATRIDGE_BULLET].get<components::missile>();
	// }

	const auto* const maybe_gun = weapon.find<components::gun>();

	if (maybe_gun) {
		return (maybe_gun->muzzle_velocity.first + maybe_gun->muzzle_velocity.second) / 2;
	}

	return 0.f;
}

ammunition_information get_ammunition_information(const const_entity_handle item) {
	ammunition_information out;

	const auto maybe_magazine_slot = item[slot_function::GUN_DETACHABLE_MAGAZINE];

	if (maybe_magazine_slot.alive() && maybe_magazine_slot.has_items()) {
		auto mag = item.get_cosmos()[maybe_magazine_slot.get_items_inside()[0]];
		auto ammo_depo = mag[slot_function::ITEM_DEPOSIT];
		out.total_charges += count_charges_in_deposit(mag);

		out.total_ammunition_space_available += ammo_depo->space_available;
		out.total_lsa += ammo_depo.calculate_local_space_available();
	}

	const auto chamber_slot = item[slot_function::GUN_CHAMBER];

	if (chamber_slot.alive()) {
		out.total_charges += count_charges_inside(chamber_slot);

		out.total_ammunition_space_available += chamber_slot->space_available;
		out.total_lsa += chamber_slot.calculate_local_space_available();
	}

	return out;
}

entity_id get_closest_hostile(
	const const_entity_handle subject,
	const const_entity_handle subject_attitude,
	const float radius,
	const b2Filter filter
) {
	const auto& cosmos = subject.get_cosmos();
	const auto si = cosmos.get_si();

	const auto& physics = cosmos.inferential_systems.get<physics_system>();
	const auto transform = subject.get_logic_transform();

	entity_id closest_hostile;

	float min_distance = std::numeric_limits<float>::max();

	if (subject_attitude.alive()) {
		const auto subject_attitude_transform = subject_attitude.get_logic_transform();

		physics.for_each_in_aabb(
			si,
			transform.pos - vec2(radius, radius),
			transform.pos + vec2(radius, radius),
			filter,
			[&](const auto fix) {
				const auto s = cosmos[get_body_entity_that_owns(fix)];

				if (s != subject && s.has<components::attitude>()) {
					const auto calculated_attitude = calculate_attitude(s, subject_attitude);

					if (is_hostile(calculated_attitude)) {
						const auto dist = (s.get_logic_transform().pos - subject_attitude_transform.pos).length_sq();

						if (dist < min_distance) {
							closest_hostile = s;
							min_distance = dist;
						}
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	return closest_hostile;
}

std::vector<entity_id> get_closest_hostiles(
	const const_entity_handle subject,
	const const_entity_handle subject_attitude,
	const float radius,
	const b2Filter filter
) {
	const auto& cosmos = subject.get_cosmos();
	const auto si = cosmos.get_si();

	const auto& physics = cosmos.inferential_systems.get<physics_system>();
	const auto transform = subject.get_logic_transform();

	struct hostile_entry {
		entity_id s;
		float dist = 0.f;

		bool operator<(const hostile_entry& b) const {
			return dist < b.dist;
		}

		bool operator==(const hostile_entry& b) const {
			return s == b.s;
		}

		operator entity_id() const {
			return s;
		}
	};

	std::vector<hostile_entry> hostiles;

	float min_distance = std::numeric_limits<float>::max();

	if (subject_attitude.alive()) {
		const auto subject_attitude_transform = subject_attitude.get_logic_transform();

		physics.for_each_in_aabb(
			si,
			transform.pos - vec2(radius, radius),
			transform.pos + vec2(radius, radius),
			filter,
			[&](const auto fix) {
				const auto s = cosmos[get_body_entity_that_owns(fix)];

				if (s != subject && s.has<components::attitude>()) {
					const auto calculated_attitude = calculate_attitude(s, subject_attitude);

					if (is_hostile(calculated_attitude)) {
						const auto dist = (s.get_logic_transform().pos - subject_attitude_transform.pos).length_sq();
						
						hostile_entry new_entry;
						new_entry.s = s;
						new_entry.dist = dist;

						hostiles.push_back(new_entry);
					}
				}

				return callback_result::CONTINUE;
			}
		);
	}

	sort_container(hostiles);
	remove_duplicates_from_sorted(hostiles);

	return { hostiles.begin(), hostiles.end() };
}