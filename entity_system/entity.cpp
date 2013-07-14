#pragma once
#include "entity.h"
#include "signature_matcher.h"
#include <cassert>

namespace augmentations {
	namespace entity_system {
		entity::entity(world& owner_world) : owner_world(owner_world) {}
		entity::~entity() { clear(); }

		std::vector<registered_type> entity::get_components() const {
			return owner_world.component_library.get_registered_types(*this);
		}

		void entity::clear() {
			/* user may have already removed all components using remove<type> calls but anyway world calls this function during deletion */
			if(type_to_component.empty()) 
				return;
			
			/* iterate through systems and remove references to this entity */
			signature_matcher_bitset my_signature(get_components());
			for(auto sys = owner_world.systems.begin(); sys != owner_world.systems.end(); ++sys) {
				if((*sys)->components_signature.matches(my_signature));
					(*sys)->remove(this);
			}

			for(auto type = type_to_component.begin(); type != type_to_component.end(); ++type) {
				/* delete component from corresponding pool, we must get component's size from the library */
				((*type).second)->~component();
				owner_world.get_container_for_type((*type).first).free((*type).second);
			}
		}
	}
}