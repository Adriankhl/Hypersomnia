#pragma once
#include <unordered_map>
#include "augs/templates/introspect.h"

namespace augs {
	template <class Enum>
	auto& get_string_to_enum_map() {
		static auto enums = []() {
			std::unordered_map<std::string, Enum> output;

			if constexpr(has_for_each_enum_v<Enum>) {
				for_each_enum([&output](const Enum e) {
					output[enum_to_string(e)] = e;
				});
			}
			else {
				for (std::size_t i = 0; i < static_cast<std::size_t>(Enum::COUNT); ++i) {
					const auto e = static_cast<Enum>(i);
					output[enum_to_string(e)] = e;
				}
			}

			return output;
		}();

		return enums;
	}
}