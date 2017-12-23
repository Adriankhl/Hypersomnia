#pragma once
#include "augs/misc/timing/stepped_timing.h"
#include "game/transcendental/entity_id.h"

struct cosmos_clock {
	// GEN INTROSPECTOR struct cosmos_clock
	augs::delta delta = augs::delta::steps_per_second(60);
	augs::stepped_timestamp now = 0;

	entity_guid next_entity_guid = 1;
	// END GEN INTROSPECTOR
};