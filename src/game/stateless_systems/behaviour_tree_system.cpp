#include "behaviour_tree_system.h"

#include "game/transcendental/cosmos.h"
#include "game/transcendental/entity_id.h"
#include "augs/ensure.h"

#include "game/components/behaviour_tree_component.h"

#include "game/transcendental/entity_handle.h"
#include "game/transcendental/logic_step.h"

using namespace augs;

void behaviour_tree_system::evaluate_trees(const logic_step) {
#if TODO
	auto& cosmos = step.get_cosmos();

	cosmos.for_each_having<components::behaviour_tree>( 
		[&](const auto it) {
			auto& behaviour_tree = it.template get<components::behaviour_tree>();
			
			for (auto& concurrent_tree : behaviour_tree.concurrent_trees) {
				// TODO: fix behaviour tree storage
				auto& tree = cosmos[concurrent_tree.tree_id];
				tree.evaluate_instance_of_tree(step, it, concurrent_tree.state);
			}
		}
	);
#endif
}