#include "game/detail/spells/spell_logic_input.h"
#include "game/transcendental/cosmos.h"

entity_handle spell_logic_input::get_subject() const {
	return step.get_cosmos()[subject];
}
