#pragma once
#include "game/cosmos/step_declaration.h"

class demolitions_system {
public:
	void detonate_fuses(const logic_step step);
	void advance_cascade_explosions(const logic_step step);
};
