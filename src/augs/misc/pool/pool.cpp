#if BUILD_UNIT_TESTS
#include <catch.hpp>
#include "augs/misc/pool/pool.h"
#include "augs/misc/constant_size_vector.h"
#include "augs/readwrite/readwrite_test_cycle.h"

using p_t = augs::pool<int, of_size<6>::make_constant_vector, unsigned short>;
using k_t = p_t::key_type; 
using u_t = p_t::undo_free_input_type; 

using uv_t = std::vector<u_t>;
using kv_t = std::vector<k_t>;

TEST_CASE("Pools SimpleBackAndForth") {
	p_t p = 6;
	kv_t keys;
	keys.resize(6);

	for (int i = 0; i < 3; ++i) {
		REQUIRE(p.empty());

		for (unsigned i = 0; i < keys.size(); ++i) {
			keys[i] = p.allocate(i).key;
		}

		REQUIRE(p.full());

		for (unsigned i = 0; i < keys.size(); ++i) {
			REQUIRE(p.get(keys[i]) == i);
		}

		for (unsigned i = 0; i < keys.size(); ++i) {
			REQUIRE(p.get(keys[i]) == i);
			p.free(keys[i]);
		}

		REQUIRE(p.empty());
	}
}

TEST_CASE("Pools UndoDeletes") {
	p_t p = 6;
	kv_t keys;
	uv_t undos;

	keys.resize(6);
	undos.resize(6);

	for (int i = 0; i < 3; ++i) {
		REQUIRE(p.empty());

		for (unsigned i = 0; i < keys.size(); ++i) {
			keys[i] = p.allocate(i).key;
		}

		REQUIRE(p.full());

		// forward direction frees
		{
			for (unsigned i = 0; i < keys.size(); ++i) {
				REQUIRE(p.get(keys[i]) == i);
				undos[i] = *p.free(keys[i]);
			}

			REQUIRE(p.empty());

			for (int idx = static_cast<int>(keys.size()) - 1; idx >= 0; --idx) {
				unsigned i = static_cast<unsigned>(idx);

				p.undo_free(undos[i], i);
				REQUIRE(p.get(keys[i]) == i);
			}
		}
			
		// reverse direction frees
		{
			for (int idx = static_cast<int>(keys.size()) - 1; idx >= 0; --idx) {
				unsigned i = static_cast<unsigned>(idx);

				REQUIRE(p.get(keys[i]) == i);
				undos[i] = *p.free(keys[i]);
			}

			REQUIRE(p.empty());

			for (unsigned i = 0; i < keys.size(); ++i) {
				p.undo_free(undos[i], i);
				REQUIRE(p.get(keys[i]) == i);
			}
		}

		/* cleanup */
		for (unsigned i = 0; i < keys.size(); ++i) {
			REQUIRE(p.get(keys[i]) == i);
			p.free(keys[i]);
		}
	}
}

template <class T>
void test_pool() {
	T p;

	readwrite_test_cycle(p);
	p.allocate(1);
	readwrite_test_cycle(p);
	p.allocate(2);
	readwrite_test_cycle(p);
	p.allocate(33);
	readwrite_test_cycle(p);

	auto id = p.allocate(1);
	readwrite_test_cycle(p);
	auto id2 = p.allocate(3);
	readwrite_test_cycle(p);
	auto id3 = p.allocate(3);
	readwrite_test_cycle(p);
	auto id4 = p.allocate(3);
	readwrite_test_cycle(p);
	p.free(id2);
	readwrite_test_cycle(p);
	p.free(id4);
	readwrite_test_cycle(p);

	REQUIRE(p.find(id4) == nullptr);
	REQUIRE(p.find(id2) == nullptr);
	REQUIRE(p.find(id) != nullptr);
	REQUIRE(p.find(id3) != nullptr);

	REQUIRE(5 == p.size());
}

TEST_CASE("Pools Readwrite") {
	test_pool<augs::pool<float, of_size<100>::make_constant_vector, unsigned short>>();
	test_pool<augs::pool<float, make_vector, unsigned char>>();
}

#endif
