#pragma once
#include "augs/misc/typesafe_sprintf.h"

struct error_with_typesafe_sprintf : public std::runtime_error {
	using std::runtime_error::what;

	template <class... Args>
	explicit error_with_typesafe_sprintf(Args&&... what)
		: std::runtime_error(typesafe_sprintf(std::forward<Args>(what)...))
	{}
};