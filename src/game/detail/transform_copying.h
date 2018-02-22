#pragma once
#include <variant>
#include <optional>
#include "game/transcendental/entity_id.h"
#include "game/components/transform_component.h"

struct absolute_or_local {
	// GEN INTROSPECTOR struct absolute_or_local
	entity_id target;
	components::transform offset;
	// END GEN INTROSPECTOR

	template <class S, class C, class I>
	static std::optional<components::transform> find_transform_impl(S& self, C& cosm, I& interp) {
		if (self.target.is_set()) {
			const auto target_handle = cosm[self.target];

			if (target_handle) {
				if (const auto target_transform = target_handle.find_viewing_transform(interp)) {
					return *target_transform * self.offset;
				}
			}

			return std::nullopt;
		}

		return self.offset;
	}

	template <class C, class I>
	auto find_transform(C& cosm, I& interp) {
		return find_transform_impl(*this, cosm, interp);
	}

	template <class C, class I>
	auto find_transform(C& cosm, I& interp) const {
		return find_transform_impl(*this, cosm, interp);
	}

	bool operator==(const absolute_or_local b) const {
		return target == b.target && offset == b.offset;
	}
};

template <class C, class... I>
std::optional<components::transform> find_transform(const absolute_or_local& l, C& cosm, I&&... interp) {
	return l.find_transform(cosm, std::forward<I>(interp)...);
}	

inline auto get_chased(const absolute_or_local& l) {
	return l.target;
}

namespace std {
	template <>
	struct hash<absolute_or_local> {
		std::size_t operator()(const absolute_or_local t) const {
			return augs::simple_two_hash(t.target, t.offset);
		}
	};
}
