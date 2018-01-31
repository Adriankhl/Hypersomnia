#pragma once
#include "augs/templates/maybe_const.h"

template <class, class>
class component_synchronizer;

template <class, class>
class synchronizer_base;

class write_synchronized_component_access {
	template <class, class>
	friend class synchronizer_base;

	template <class, class>
	friend class component_synchronizer;

	write_synchronized_component_access() {}
};

template <class entity_handle_type, class component_type>
class synchronizer_base {
protected:
	/*
		A value of nullptr means that the entity has no such component.
	*/

	static constexpr bool is_const = is_class_const_v<entity_handle_type>;
	using component_pointer = maybe_const_ptr_t<is_const, component_type>;

	component_pointer component;
	entity_handle_type handle;

public:
	auto& get_raw_component(write_synchronized_component_access) const {
		return *component;
	}

	const auto& get_raw_component() const {
		return *component;
	}

	auto get_handle() const {
		return handle;
	}

	bool operator==(const std::nullptr_t) const {
		return component == nullptr;
	}

	bool operator!=(const std::nullptr_t) const {
		return component != nullptr;
	}

	operator bool() const {
		return component != nullptr;
	}

	auto* operator->() const {
		return this;
	}

	synchronizer_base(
		const component_pointer c, 
		const entity_handle_type h
	) : 
		component(c), 
		handle(h)
	{}
};
