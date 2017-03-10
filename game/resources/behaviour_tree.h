#pragma once
#include <vector>
#include <memory>
#include <tuple>
#include <array>

#include "game/transcendental/entity_id.h"
#include "game/detail/ai/goals.h"

#include "augs/ensure.h"
#include "game/transcendental/entity_handle.h"

#include "game/transcendental/logic_step.h"

namespace resources {
	struct state_of_behaviour_tree_instance {
		// GEN INTROSPECTOR struct resources::state_of_behaviour_tree_instance
		int previously_executed_leaf_id = -1;
		// END GEN INTROSPECTOR
	};

	class behaviour_tree {
	public:
		enum class goal_availability {
			ALREADY_ACHIEVED,
			CANT_EXECUTE,
			SHOULD_EXECUTE
		};

		enum class execution_occurence {
			FIRST,
			LAST,
			REPEATED
		};

		struct state_of_traversal {
			state_of_traversal(
				const logic_step, 
				const entity_handle, 
				state_of_behaviour_tree_instance&, 
				const behaviour_tree&
			);

			const logic_step step;
			const entity_handle subject;
			state_of_behaviour_tree_instance& instance;
			const behaviour_tree& original_tree;
			
			behaviours::goal_tuple resolved_goals;
			std::array<bool, std::tuple_size_v<decltype(resolved_goals)>> goals_set;

			template<class T>
			void set_goal(const T& g) {
				std::get<T>(resolved_goals) = g;
				goals_set.at(index_in_list_v<T, decltype(resolved_goals)>) = true;
			}

			template<class T>
			T& get_goal() {
				ensure(goals_set.at(index_in_list_v<T, decltype(resolved_goals)>));
				return std::get<T>(resolved_goals);
			}
		};

		class node {
			int this_id_in_tree = -1;
			friend class behaviour_tree;

			goal_availability evaluate_node(state_of_traversal&) const;
		public:
			enum class type {
				SEQUENCER,
				SELECTOR,
			} mode = type::SELECTOR;

			std::vector<std::unique_ptr<node>> children;

			node* create_branches();

			template<typename Branch, typename... Branches>
			node* create_branches(Branch b, Branches... branches) {
				children.emplace_back(b);
				return create_branches(branches...);
			}

			virtual goal_availability goal_resolution(state_of_traversal&) const;
			virtual void execute_leaf_goal_callback(execution_occurence, state_of_traversal&) const;
		};

		node root;
		void build_tree();
		
		void evaluate_instance_of_tree(
			const logic_step, 
			const entity_handle, 
			state_of_behaviour_tree_instance&
		) const;

		const node& get_node_by_id(const int) const;

	private:
		std::vector<const node*> tree_nodes;

		template<class F>
		void call_on_node_recursively(node& p, const F f) {
			f(p);

			for (auto& child : p.children) {
				call_on_node_recursively(*child, f);
			}
		}
	};
}
