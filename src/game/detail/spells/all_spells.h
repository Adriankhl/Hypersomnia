#pragma once
#include "augs/templates/transform_types.h"
#include "augs/templates/instance_type.h"
#include "augs/templates/type_in_list_id.h"

#include "game/detail/spells/haste.h"
#include "game/detail/spells/fury_of_the_aeons.h"
#include "game/detail/spells/ultimate_wrath_of_the_aeons.h"
#include "game/detail/spells/electric_shield.h"
#include "game/detail/spells/electric_triad.h"
#include "game/detail/spells/exaltation.h"
#include "game/detail/spells/echoes_of_the_higher_realms.h"

template <template <class...> class List>
using spell_list_t = List<
	haste,
	fury_of_the_aeons,
	ultimate_wrath_of_the_aeons,
	electric_shield,
	electric_triad,
	exaltation,
	echoes_of_the_higher_realms
>;

template <template <class...> class List>
using spell_instance_list_t = transform_types_in_list_t<
	spell_list_t<List>,
	instance_of
>;

using spell_instance_tuple = spell_instance_list_t<augs::trivially_copyable_tuple>;
using spell_id = type_in_list_id<spell_instance_tuple>;