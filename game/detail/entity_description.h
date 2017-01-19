#pragma once
#include <string>
#include "game/enums/entity_name.h"
#include "game/transcendental/entity_id.h"

#include "game/detail/inventory_slot_handle_declaration.h"
#include "game/transcendental/entity_handle_declaration.h"
#include "game/enums/slot_function.h"
#include "game/enums/item_category.h"

struct textual_description {
	std::wstring name;
	std::wstring details;
} description_by_entity_name(const entity_name),
description_of_entity(const const_entity_handle)
;

std::wstring describe_properties(const const_entity_handle);

std::wstring describe_item_compatibility_categories(const item_category_bitset& flags);
textual_description describe_slot_function(const slot_function);

std::wstring describe_slot(const const_inventory_slot_handle&);
std::wstring describe_entity(const const_entity_handle);
