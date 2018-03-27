#pragma once
#include "augs/templates/introspect.h"
#include "augs/templates/string_templates_declaration.h"

template <class Enum>
auto format_enum(const Enum e) {
	return format_field_name(to_lowercase(augs::enum_to_string(e)));
}