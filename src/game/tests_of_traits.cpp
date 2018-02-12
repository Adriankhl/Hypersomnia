#include "augs/filesystem/path.h"

#include "augs/templates/is_comparable.h"
#include "augs/templates/type_map.h"
#include "augs/readwrite/custom_lua_representations.h"

#include "augs/readwrite/memory_stream.h"

#include "game/transcendental/cosmos.h"
#include "game/organization/all_component_includes.h"

#include "augs/templates/predicate_templates.h"
#include "augs/templates/get_index_type_for_size_of.h"

#include "game/assets/all_logical_assets.h"

#include "augs/templates/container_traits.h"
#include "game/components/pathfinding_component.h"

#include "augs/pad_bytes.h"

#include "augs/readwrite/lua_readwrite.h"
#include "augs/readwrite/byte_readwrite.h"

#include "3rdparty/imgui/imgui.h"

// shortcut
template <class A, class B>
constexpr bool same = std::is_same_v<A, B>;

template <typename Trait>
struct size_test_detail {
	static Trait ttt;

	template<int Value = ttt.size()>
	static std::true_type do_call(int) { return std::true_type(); }

	static std::false_type do_call(...) { return std::false_type(); }

	static auto call() { return do_call(0); }
};

template <typename Trait>
struct size_test : decltype(size_test_detail<Trait>::call()) {};

template <class T>
constexpr bool is_constexpr_size_v = size_test<T>::value;

namespace templates_detail {
	template <class T>
	struct identity {
		using type = T;
	};

	template <class T>
	using identity_t = typename identity<T>::type;
}

struct AAA {
	int& czo;
};

static void ff() {
	all_entity_types t;

	auto okay = get_by_dynamic_index(t, std::size_t(0), [](auto a){
		return 20.0;	
	});

	auto okay2 = get_by_dynamic_id(t, type_in_list_id<all_entity_types>(), [](auto a){
		return 20.0;	
	});

	using candidates = type_list<plain_missile, explosive_missile>;

	auto tester = [](auto a) -> decltype(auto) {
		using T = std::decay_t<decltype(a)>;
		static_assert(same<T, plain_missile> || same<T, explosive_missile>);
		return 20.0;	
	};

	auto okay3 = conditional_get_by_dynamic_index<candidates>(t, std::size_t(0), tester);
	auto okay4 = conditional_get_by_dynamic_id<candidates>(t, type_in_list_id<all_entity_types>(), tester);

	static_assert(same<double, decltype(okay)>);
	static_assert(same<double, decltype(okay2)>);
	static_assert(same<double, decltype(okay3)>);
	static_assert(same<double, decltype(okay4)>);
}

static void gg() {
	{
		using tp = type_map<
			type_pair<int, double>,
			type_pair<double, int>,
			type_pair<float, const char*>,
			type_pair<const int, std::string>
		>;

		static_assert(same<tp::at<int>, double>);
		static_assert(same<tp::at<double>, int>);
		static_assert(same<tp::at<float>, const char*>);
		static_assert(same<tp::at<const int>, std::string>);
	}
	typed_entity_id<controlled_character>() == typed_entity_id<controlled_character>();
}
struct tests_of_traits {
	static_assert(is_comparable_v<const int, int>);
	static_assert(!is_comparable_v<typed_entity_id<controlled_character>, typed_entity_id<plain_missile>>);

	static_assert(has_any_of_v<controlled_character, invariants::sprite, invariants::polygon>);
	static_assert(!has_any_of_v<controlled_character, invariants::trace>);

	static_assert(has_invariants_or_components_v<controlled_character, components::interpolation>);
	static_assert(!has_invariants_or_components_v<plain_invisible_body, components::interpolation>);

	//static_assert(std::is_trivially_copyable_v<absolute_or_local>);
	static_assert(same<double, type_argument_t<std::is_trivially_copyable<double>>>);
	static_assert(same<constrained_entity_flavour_id<invariants::missile>::matching_types, type_list<plain_missile, explosive_missile>>);

	static_assert(has_specific_entity_type_v<typed_entity_handle<controlled_character>>);
	static_assert(!has_specific_entity_type_v<const_entity_handle>);

	static_assert(all_are_v<std::is_trivially_copyable, type_list<int, double, float>>);

	static_assert(same<
		type_list<int, int, double, double>,
		concatenate_lists_t<type_list<int, int>, type_list<double, double>>
	>);

	static_assert(same<
		std::tuple<int, int, double, double>,
		concatenate_lists_t<std::tuple<int, int>, type_list<double, double>>
	>);

	static_assert(is_handle_const_v<const_entity_handle>);
	static_assert(!is_handle_const_v<iterated_entity_handle<controlled_character>>);
	static_assert(is_handle_const_v<const_iterated_entity_handle<controlled_character>>);

	static_assert(!can_reserve_caches_v<flavour_id_cache>);
	static_assert(can_reserve_caches_v<physics_world_cache>);
	static_assert(std::is_trivially_copyable_v<game_intent_type>);
	static_assert(has_string_v<augs::path_type>);
	static_assert(has_wstring_v<augs::path_type>);

	static_assert(static_cast<int>(imguicol_helper::ImGuiCol_COUNT) == static_cast<int>(ImGuiCol_COUNT));

	static_assert(augs::has_byte_readwrite_overloads_v<augs::memory_stream, augs::path_type>);
	static_assert(augs::has_byte_readwrite_overloads_v<augs::memory_stream, cosmos>);
	static_assert(augs::has_lua_readwrite_overloads_v<cosmos>);

	static_assert(b2_maxPolygonVertices == CONVEX_POLY_VERTEX_COUNT);

	static_assert(
		sizeof(entity_id) >= sizeof(entity_guid),
		"With given memory layouts, entity_id<->entity_guid substitution will not be possible in delta encoding"
	);

	static_assert(is_container_v<augs::path_type>);
	static_assert(!is_padding_field_v<entity_id>);
	static_assert(is_padding_field_v<pad_bytes<4>>);
	static_assert(is_padding_field_v<pad_bytes<1>>);

	static_assert(!has_introspect_v<cosmos>, "Trait has failed");
	static_assert(has_introspect_v<cosmos_clock>, "Trait has failed");
	//static_assert(has_introspect_v<augs::constant_size_vector<int, 2>>, "Trait has failed");
	static_assert(has_introspect_v<zeroed_pod<unsigned int>>, "Trait has failed");
	static_assert(has_introspect_v<augs::delta>, "Trait has failed");
	static_assert(alignof(meter_instance_tuple) == 4, "Trait has failed");

	static_assert(same<std::tuple<int, double, float>, reverse_types_in_list_t<std::tuple<float, double, int>>>, "Trait has failed");
	static_assert(same<type_list<int, double, float>, reverse_types_in_list_t<type_list<float, double, int>>>, "Trait has failed");

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

	static_assert(!is_constexpr_size_v<std::vector<int>>, "Trait has failed");
	static_assert(is_constexpr_size_v<std::array<int, 3>>, "Trait has failed");
	static_assert(is_constexpr_size_v<std::array<vec2, 3>>, "Trait has failed");
	static_assert(is_constexpr_size_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");

	static_assert(is_constexpr_capacity_v<augs::constant_size_vector<int, 20>>, "Trait has failed");
	static_assert(!is_constexpr_capacity_v<std::vector<int>>, "Trait has failed");

	static_assert(!is_container_v<decltype(pad_bytes<3>::pad)>, "Trait has failed");
	static_assert(is_container_v<augs::enum_associative_array<game_intent_type, vec2>>, "Trait has failed");
	static_assert(!is_container_v<augs::enum_array<basic_transform<float>, colliders_offset_type>>, "Trait has failed");

	static_assert(is_container_v<augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, augs::constant_size_vector<vec2, 20>>, "Trait has failed");
	static_assert(augs::is_byte_readwrite_appropriate_v<augs::memory_stream, augs::enum_associative_array<game_intent_type, vec2>>, "Trait has failed");
	static_assert(is_container_v<std::vector<int>>, "Trait has failed");
	static_assert(is_container_v<std::vector<vec2>>, "Trait has failed");
	static_assert(is_container_v<std::vector<cosmos>>, "Trait has failed");
	static_assert(is_container_v<std::vector<pathfinding_session>>, "Trait has failed");

	static_assert(can_access_data_v<std::string>, "Trait has failed");
	static_assert(can_access_data_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_access_data_v<std::set<int>>, "Trait has failed");
	static_assert(can_reserve_v<std::vector<int>>, "Trait has failed");
	static_assert(!can_reserve_v<std::map<int, int>>, "Trait has failed");

	static_assert(!has_introspect_v<unsigned>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<float>>, "Trait has failed");
	static_assert(has_introspect_v<basic_ltrb<int>>, "Trait has failed");

	static_assert(bind_types<std::is_same, const int>::type<const int>::value, "Trait has failed");

	static_assert(same<filter_types_in_list<std::is_integral, type_list<double, int, float>>::indices, std::index_sequence<1>>, "Trait has failed");
	static_assert(same<filter_types_in_list<std::is_integral, type_list<double, int, float>>::types, type_list<int>>, "Trait has failed");
	static_assert(same<filter_types_in_list<std::is_integral, type_list<double, int, float>>::get_type<0>, int>, "Trait has failed");
	
	static_assert(is_one_of_list_v<unsigned, std::tuple<float, float, double, unsigned>>, "Trait has failed");
	static_assert(!is_one_of_v<int, float, double>, "Trait has failed");
	static_assert(is_one_of_v<unsigned, float, float, double, unsigned>, "Trait has failed");
	static_assert(is_one_of_v<cosmos, int, cosmos_clock, cosmos>, "Trait has failed");

	static_assert(index_in_list_v<unsigned, std::tuple<float, float, double, unsigned>> == 3, "Trait has failed");
	static_assert(index_in_v<unsigned, float, float, double, unsigned> == 3, "Trait has failed");
	
	static_assert(same<unsigned, nth_type_in_t<0, unsigned, float, float>>, "Trait has failed");
	static_assert(same<float, nth_type_in_t<1, unsigned, float, float>>, "Trait has failed");
	static_assert(same<float, nth_type_in_t<2, unsigned, float, float>>, "Trait has failed");
	static_assert(same<double, nth_type_in_t<3, unsigned, float, float, double, unsigned>>, "Trait has failed");
	
	static_assert(
		same<
			filter_types_in_list<std::is_integral, type_list<int, double, float, unsigned>>::types, 
			type_list<int, unsigned>
		>, 
		"Trait has failed"
	);
	
	static_assert(
		same<
		filter_types_in_list<std::is_integral, type_list<int, double, float, unsigned>>::indices,
			std::index_sequence<0, 3>
		>, 
		"Trait has failed"
	);
	
	static_assert(
		!same<
		filter_types_in_list<std::is_floating_point, type_list<int, double, float, unsigned>>::types,
			std::tuple<int, unsigned>
		>, 
		"Trait has failed"
	);

	static_assert(
		same<type_list<int&, double&, float&>, transform_types_in_list_t<type_list<int, double, float>, std::add_lvalue_reference_t>>,
		"Trait has failed."
	);
	
	static_assert(
		same<type_list<int, double, float>, transform_types_in_list_t<type_list<const int&, double&&, float&>, std::decay_t>>,
		"Trait has failed."
	);
	
	static_assert(
		same<type_list<const int&, double&&, float&>, transform_types_in_list_t<type_list<const int&, double&&, float&>, templates_detail::identity_t>>,
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

	static_assert(same<unsigned char, get_index_type_for_size_of_t<A>>, "Trait has failed");
	static_assert(same<unsigned char, get_index_type_for_size_of_t<B>>, "Trait has failed");
	static_assert(same<unsigned char, get_index_type_for_size_of_t<C>>, "Trait has failed");
	static_assert(same<unsigned short, get_index_type_for_size_of_t<D>>, "Trait has failed");
	static_assert(same<unsigned short, get_index_type_for_size_of_t<E>>, "Trait has failed");
	static_assert(same<unsigned int, get_index_type_for_size_of_t<F>>, "Trait has failed");

	//static_assert(sizeof(cosmos) < 1000000, "Possible stack overflow due to cosmos on the stack");

	static_assert(is_introspective_leaf_v<launch_type>);
	static_assert(has_enum_to_string_v<launch_type>);
	static_assert(has_enum_to_string_v<launch_type>);
	static_assert(has_for_each_enum_v<launch_type>);
	static_assert(has_for_each_enum_v<input_recording_type>);

	static_assert(augs::has_custom_to_lua_value_v<augs::path_type>);
	static_assert(augs::has_custom_to_lua_value_v<rgba>);
	static_assert(augs::has_custom_to_lua_value_v<ImVec4>);
	static_assert(augs::has_custom_to_lua_value_v<std::wstring>);
	static_assert(augs::representable_as_lua_value_v<std::wstring>);
	static_assert(augs::representable_as_lua_value_v<const std::wstring*>);
	
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
