#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"

#include "augs/templates/predicate_templates.h"
#include "augs/templates/get_index_type_for_size_of.h"

#include "generated/introspectors.h"
#include "game/assets/all_logical_assets.h"

#include "augs/readwrite/byte_readwrite.h"
#include "augs/templates/container_traits.h"
#include "game/components/pathfinding_component.h"

#include "augs/pad_bytes.h"
#include "augs/misc/custom_lua_representations.h"
#include "augs/readwrite/lua_readwrite.h"

namespace templates_detail {
	template <class T>
	struct identity {
		using type = T;
	};

	template <class T>
	using identity_t = typename identity<T>::type;
}

struct tests_of_traits {
	static_assert(b2_maxPolygonVertices == CONVEX_POLY_VERTEX_COUNT);

	static_assert(
		sizeof(entity_id) >= sizeof(entity_guid),
		"With given memory layouts, entity_id<->entity_guid substitution will not be possible in delta encoding"
	);

	static_assert(is_component_fundamental_v<components::name>);

	static_assert(is_container_v<augs::path_type>);
	static_assert(!is_padding_field_v<entity_id>);
	static_assert(is_padding_field_v<pad_bytes<4>>);
	static_assert(is_padding_field_v<pad_bytes<1>>);

	static_assert(!has_introspect_v<cosmos>, "Trait has failed");
	static_assert(has_introspect_v<cosmos_metadata>, "Trait has failed");
	static_assert(has_introspect_v<augs::constant_size_vector<int, 2>>, "Trait has failed");
	static_assert(has_introspect_v<zeroed_pod<unsigned int>>, "Trait has failed");
	static_assert(has_introspect_v<augs::delta>, "Trait has failed");
	static_assert(alignof(meter_instance_tuple) == 4, "Trait has failed");

	static_assert(std::is_same_v<std::tuple<int, double, float>, reverse_types_in_list_t<std::tuple<float, double, int>>>, "Trait has failed");
	static_assert(std::is_same_v<type_list<int, double, float>, reverse_types_in_list_t<type_list<float, double, int>>>, "Trait has failed");

	static_assert(sum_sizes_until_nth_v<0, std::tuple<int, double, float>> == 0, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<1, std::tuple<int, double, float>> == 4, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<2, std::tuple<int, double, float>> == 12, "Trait has failed");
	static_assert(sum_sizes_until_nth_v<3, std::tuple<int, double, float>> == 16, "Trait has failed");

	static_assert(sum_sizes_of_types_in_list_v<std::tuple<int, double, float>> == 16, "Trait has failed");
	static_assert(sum_sizes_of_types_in_list_v<std::tuple<int>> == 4, "Trait has failed");
	static_assert(sum_sizes_of_types_in_list_v<std::tuple<>> == 0, "Trait has failed");

	static_assert(count_occurences_in_v<int, int, double, float> == 1, "Trait has failed");
	static_assert(count_occurences_in_list_v<int, std::tuple<int, double, float>> == 1, "Trait has failed");
	static_assert(count_occurences_in_list_v<int, std::tuple<int, double, float, int>> == 2, "Trait has failed");

	static_assert(!is_constexpr_size_container_v<std::vector<int>>, "Trait has failed");
	static_assert(is_constexpr_size_container_v<std::array<int, 3>>, "Trait has failed");
	static_assert(is_constexpr_size_container_v<std::array<vec2, 3>>, "Trait has failed");
	static_assert(is_constexpr_size_container_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");
	static_assert(!is_variable_size_container_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");
	static_assert(is_variable_size_container_v<augs::enum_associative_array<game_intent_type, vec2>>, "Trait has failed");

	
	static_assert(is_unary_container_v<augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(is_variable_size_container_v<augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::stream, augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::stream, augs::enum_associative_array<game_intent_type, vec2>>, "Trait has failed");
	static_assert(is_variable_size_container_v<std::vector<int>>, "Trait has failed");
	static_assert(is_variable_size_container_v<std::vector<vec2>>, "Trait has failed");
	static_assert(is_variable_size_container_v<std::vector<cosmos>>, "Trait has failed");
	static_assert(is_variable_size_container_v<std::vector<pathfinding_session>>, "Trait has failed");

	static_assert(can_access_data_v<std::string>, "Trait has failed");
	static_assert(can_access_data_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_access_data_v<std::set<int>>, "Trait has failed");
	static_assert(can_reserve_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_reserve_v<std::map<int, int>>, "Trait has failed");

	static_assert(!has_introspect_v<unsigned>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<float>>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<int>>, "Trait has failed");

	static_assert(bind_types<std::is_same, const int>::type<const int>::value, "Trait has failed");

	static_assert(std::is_same_v<filter_types_in_list<std::is_integral, type_list<double, int, float>>::indices, std::index_sequence<1>>, "Trait has failed");
	static_assert(std::is_same_v<filter_types_in_list<std::is_integral, type_list<double, int, float>>::type, std::tuple<int>>, "Trait has failed");
	static_assert(std::is_same_v<filter_types_in_list<std::is_integral, type_list<double, int, float>>::get_type<0>::type, int>, "Trait has failed");
	
	static_assert(is_one_of_list_v<unsigned, std::tuple<float, float, double, unsigned>>, "Trait has failed");
	static_assert(!is_one_of_v<int, float, double>, "Trait has failed");
	static_assert(is_one_of_v<unsigned, float, float, double, unsigned>, "Trait has failed");
	static_assert(is_one_of_v<cosmos, int, cosmos_metadata, cosmos>, "Trait has failed");

	static_assert(index_in_list_v<unsigned, std::tuple<float, float, double, unsigned>> == 3, "Trait has failed");
	static_assert(index_in_v<unsigned, float, float, double, unsigned> == 3, "Trait has failed");
	
	static_assert(std::is_same_v<unsigned, nth_type_in_t<0, unsigned, float, float>>, "Trait has failed");
	static_assert(std::is_same_v<double, nth_type_in_t<3, unsigned, float, float, double, unsigned>>, "Trait has failed");
	
	static_assert(
		std::is_same_v<
			filter_types_in_list<std::is_integral, type_list<int, double, float, unsigned>>::type, 
			std::tuple<int, unsigned>
		>, 
		"Trait has failed"
	);
	
	static_assert(
		std::is_same_v<
		filter_types_in_list<std::is_integral, type_list<int, double, float, unsigned>>::indices,
			std::index_sequence<0, 3>
		>, 
		"Trait has failed"
	);
	
	static_assert(
		!std::is_same_v<
		filter_types_in_list<std::is_floating_point, type_list<int, double, float, unsigned>>::type,
			std::tuple<int, unsigned>
		>, 
		"Trait has failed"
	);

	static_assert(
		std::is_same_v<type_list<int&, double&, float&>, transform_types_in_list_t<type_list<int, double, float>, std::add_lvalue_reference_t>>,
		"Trait has failed."
	);
	
	static_assert(
		std::is_same_v<type_list<int, double, float>, transform_types_in_list_t<type_list<const int&, double&&, float&>, std::decay_t>>,
		"Trait has failed."
	);
	
	static_assert(
		std::is_same_v<type_list<const int&, double&&, float&>, transform_types_in_list_t<type_list<const int&, double&&, float&>, templates_detail::identity_t>>,
		"Trait has failed."
	);

	struct A {
		char a;
	};

	struct B {
		char a[255];
	};

	struct C {
		char a[256];
	};

	struct D {
		char a[257];
	};

	struct E {
		char a[65536];
	};

	struct F {
		char a[65537];
	};

	static_assert(std::is_same_v<unsigned char, get_index_type_for_size_of_t<A>>, "Trait has failed");
	static_assert(std::is_same_v<unsigned char, get_index_type_for_size_of_t<B>>, "Trait has failed");
	static_assert(std::is_same_v<unsigned char, get_index_type_for_size_of_t<C>>, "Trait has failed");
	static_assert(std::is_same_v<unsigned short, get_index_type_for_size_of_t<D>>, "Trait has failed");
	static_assert(std::is_same_v<unsigned short, get_index_type_for_size_of_t<E>>, "Trait has failed");
	static_assert(std::is_same_v<unsigned int, get_index_type_for_size_of_t<F>>, "Trait has failed");

	//static_assert(sizeof(cosmos) < 1000000, "Possible stack overflow due to cosmos on the stack");

	static_assert(is_introspective_leaf_v<launch_type>);
	static_assert(has_enum_to_string_v<launch_type>);
	static_assert(has_enum_to_string_v<launch_type>);
	static_assert(has_for_each_enum_v<launch_type>);
	static_assert(has_for_each_enum_v<input_recording_type>);

	static_assert(augs::has_custom_to_lua_value_v<rgba>);
	static_assert(augs::has_custom_to_lua_value_v<ImVec4>);
	static_assert(augs::has_custom_to_lua_value_v<std::wstring>);
	static_assert(augs::representable_as_lua_value_v<std::wstring>);
	static_assert(augs::representable_as_lua_value_v<const std::wstring*>);
	
	static_assert(!augs::has_readwrite_overloads_v<augs::stream, cosmic_entity>);

	static_assert(aligned_num_of_bytes_v<0, 4> == 0, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<1, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<2, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<3, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<4, 4> == 4, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<5, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<6, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<7, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<8, 4> == 8, "Trait is wrong");
	static_assert(aligned_num_of_bytes_v<9, 4> == 12, "Trait is wrong");
};

/*
constexpr auto all_assets_size = sizeof(all_assets);
constexpr auto tuple_of_assets_size = sizeof(tuple_of_all_assets);
constexpr auto tuple_of_logical_assets_size = sizeof(tuple_of_all_logical_assets);

sizeof(augs::enum_associative_array<assets::animation_id, animation>);
sizeof(augs::enum_associative_array<assets::game_image_id, game_image_baked>);
sizeof(augs::enum_associative_array<assets::font_id, game_font_baked>);
sizeof(augs::enum_associative_array<assets::particle_effect_id, particle_effect>);
sizeof(augs::enum_associative_array<assets::physical_material_id, physical_material>);
sizeof(augs::enum_associative_array<assets::shader_id, augs::graphics::shader>);
sizeof(augs::enum_associative_array<assets::shader_program_id, augs::graphics::shader_program>);
sizeof(augs::enum_associative_array<assets::sound_buffer_id, augs::sound_buffer>);
sizeof(augs::enum_associative_array<assets::gl_texture_id, augs::graphics::texture>);
sizeof(particle_effect_logical);

static_assert(
sizeof(tuple_of_all_logical_assets) <= sizeof(tuple_of_all_assets),
"Metadatas should not be bigger than the objects themselves!"
);

static_assert(
sizeof(logical_type) <= sizeof(typename T::mapped_type),
"Metadata should not be bigger than the object itself!"
);
*/