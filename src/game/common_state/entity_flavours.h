#pragma once
#include <tuple>

#include "augs/pad_bytes.h"
#include "augs/misc/pool/pool.h"
#include "augs/templates/introspect_declaration.h"
#include "augs/templates/folded_finders.h"
#include "augs/templates/maybe_const.h"
#include "augs/templates/traits/component_traits.h"

#include "game/transcendental/entity_flavour_id.h"
#include "game/transcendental/per_entity_type.h"

#include "game/organization/all_component_includes.h"
#include "game/organization/all_entity_types_declaration.h"

template <class entity_type>
class entity_flavour {
	using invariants_type = make_invariants<entity_type>;

	template <class D, class E>
	static auto& get_impl(E& self) {
		if constexpr(is_invariant_v<D>) {
			static_assert(E::template has<D>(), "Entity type does not have this invariant.");
			return std::get<D>(self.invariants); 
		}
		else {
			static_assert(E::template has<D>(), "Unknown initial component type.");
			return std::get<D>(self.initial_components); 
		}
	}

	template <class D, class E>
	static auto find_impl(E& self) -> maybe_const_ptr_t<std::is_const_v<E>, D> {
		if constexpr(E::template has<D>()) {
			return std::addressof(std::get<D>(self.invariants));
		}

		return nullptr; 
	}

public:
	using initial_components_type = make_components<entity_type>;
	using used_entity_type = entity_type;

	// GEN INTROSPECTOR class entity_flavour class entity_type
	invariants_type invariants;
	initial_components_type initial_components;
	// END GEN INTROSPECTOR

	template <class D>
	static constexpr bool has() {
		return 
			is_one_of_list_v<D, invariants_type>
			|| is_one_of_list_v<D, initial_components_type>
		;
	}

	const auto& get_name() const {
		return get<invariants::name>().name;
	}

	const auto& get_description() const {
		return get<invariants::name>().description;
	}

	template <class D>
	D* find() {
		return find_impl<D>(*this);
	}

	template <class D>
	const D* find() const {
		return find_impl<D>(*this);
	}

	template <class D>
	D& get() {
		return get_impl<D>(*this);
	}

	template <class D>
	const D& get() const {
		return get_impl<D>(*this);
	}

	template <class D>
	void set(const D& something) {
		get<D>() = something;
	}

	bool operator==(const entity_flavour& b) const {
		return augs::introspective_equal(*this, b);
	}
};

#if STATICALLY_ALLOCATE_ENTITY_FLAVOURS
#include "augs/misc/constant_size_vector.h"

template <class E>
using entity_flavours_container = 
	augs::pool<
		entity_flavour<E>, 
		of_size<E::statically_allocated_flavours>::template make_constant_vector,
		unsigned short,
		raw_flavour_id_key
	>
;

#else
#include <vector>

template <class E>
using entity_flavours_container = 
	augs::pool<
		entity_flavour<E>, 
		make_vector
		unsigned short,
		raw_flavour_id_key
	>
;

#endif

namespace augs {
	struct introspection_access;
}

template <class E>
using make_entity_flavours = entity_flavours_container<E>;
using all_entity_flavours = per_entity_type_container<make_entity_flavours>;