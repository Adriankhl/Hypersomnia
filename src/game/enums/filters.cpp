#include "filters.h"

auto all = std::numeric_limits<decltype(b2Filter::categoryBits)>::max() & (~filters::TRIGGER);

namespace filters {
	b2Filter none() {
		b2Filter out;
		out.categoryBits = 0;
		out.maskBits = 0;
		out.groupIndex = 0;
		return out;
	}

	b2Filter renderable() {
		b2Filter out;
		out.categoryBits = RENDERABLE;
		out.maskBits = RENDERABLE_QUERY;
		return out;
	}

	b2Filter renderable_query() {
		b2Filter out;
		out.categoryBits = RENDERABLE_QUERY;
		out.maskBits = RENDERABLE;
		return out;
	}

	b2Filter pathfinding_query() {
		b2Filter out;
		out.categoryBits = PATHFINDING_QUERY;
		out.maskBits = PATHFINDING_OBSTRUCTION;
		return out;
	}
	
	b2Filter line_of_sight_query() {
		b2Filter out;
		out.categoryBits = SIGHT_QUERY;
		out.maskBits = SIGHT_OBSTRUCTION;
		return out;
	}

	b2Filter line_of_sight_candidates() {
		b2Filter out;
		out.categoryBits = RENDERABLE_QUERY;
		out.maskBits = DYNAMIC_OBJECT | BULLET;
		return out;
	}

	b2Filter controlled_character() {
		b2Filter out;
		out.categoryBits = RENDERABLE | CONTROLLED_CHARACTER | DYNAMIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | FRICTION_GROUND | TRIGGER | BULLET | SHELL;
		return out;
	}
	
	b2Filter friction_ground() {
		b2Filter out;
		out.categoryBits = RENDERABLE | FRICTION_GROUND;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | FRICTION_GROUND | SHELL;
		return out;
	}

	b2Filter dynamic_object() {
		b2Filter out;
		out.categoryBits = SIGHT_OBSTRUCTION | PATHFINDING_OBSTRUCTION | RENDERABLE | DYNAMIC_OBJECT;
		out.maskBits = SIGHT_QUERY | PATHFINDING_QUERY | RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | FRICTION_GROUND | BULLET | SHELL;
		return out;
	}

	b2Filter see_through_dynamic_object() {
		b2Filter out;
		out.categoryBits = RENDERABLE | DYNAMIC_OBJECT;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | FRICTION_GROUND | BULLET | SHELL;
		return out;
	}

	b2Filter static_object() {
		b2Filter out;
		out.categoryBits = SIGHT_OBSTRUCTION | PATHFINDING_OBSTRUCTION | RENDERABLE | STATIC_OBJECT;
		out.maskBits = SIGHT_QUERY | PATHFINDING_QUERY | RENDERABLE_QUERY | CONTROLLED_CHARACTER | DYNAMIC_OBJECT | FRICTION_GROUND | BULLET | SHELL;
		return out;
	}

	b2Filter shell() {
		b2Filter out;
		out.categoryBits = RENDERABLE | SHELL;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT | FRICTION_GROUND | SHELL;
		return out;
	}

	b2Filter bullet() {
		b2Filter out;
		out.categoryBits = RENDERABLE | BULLET;
		out.maskBits = RENDERABLE_QUERY | CONTROLLED_CHARACTER | STATIC_OBJECT | DYNAMIC_OBJECT;
		return out;
	}

	b2Filter trigger() {
		b2Filter out;
		out.categoryBits = RENDERABLE | TRIGGER;
		out.maskBits = RENDERABLE_QUERY | TRIGGER | CONTROLLED_CHARACTER;
		return out;
	}

}
