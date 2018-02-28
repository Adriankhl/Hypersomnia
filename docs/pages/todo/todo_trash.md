---
title: ToDo trash
hide_sidebar: true
permalink: todo_trash
summary: Just a hidden scratchpad.
---

	- ~~thus let on_add and on_remove be called when adding or removing components.~~
	- ~~proposed solution: if the only moment that we add or remove components is when creating an entity and when changing its type...~~
		- ~~we could ditch partial reinferences and implement only whole-entity destruction and inference~~
			- not quite, because we will still need partial reinference
			- if we do partial reinference we can as well enable it when components are added or removed

		- ~~as for dependencies, specify them in components/component synchronizers~~
		- adders and removers should always explicitly uphold the consistency of state
<!--
		- entity handle calls on_remove/on_add cycle on all dependencies after calling the subject's on_add or on_remove. Examples:
			- removing/adding an item calls on_remove/on_add for rigid body component
				- since item ownership could change
				- custom body owner is a stronger body overrider than a container
			- removing/adding a custom body owner calls on_remove/on_add for rigid body component
			- removing/adding item  
		- items_of_slots -> fixtures_of_bodies -> physics_world_cache
--!>
- replace "reinfer all caches of" calls in entity handle with something else
	- existence of components should imply these things
		- basic processing categories
		- existence of correspondent invariants
			- in particular, it should be unsafe to access invariants if component has not been checked beforehand 
		- existence of caches
			- as for always_present components which need caches...
				- ...the cache should be inferred right with creation of the entity,
				- and destroyed only upon its death.
					- or at otherwise special circumstances?
					- changing the type is pretty much special
				- makes sense for guid and type components.


	- problem: if container definition was enabled, container behaviour will be present even if we destroy all components, thus breaking our assumptions that destroying all caches via removing all components.
		- provide a container component with activated flag?
			- no activation flag. Just an empty component.
			- even if there is no circumstantial data that we need now, it may change in the future
		- provide per-entity cache destroyer that takes invariants into account?
		- make type dynamic but statically allocated and let it handle that?
	- especially since a component removed in one entity may trigger change of caches for other entities, dependencies will not be as easy to determine as specifying some types
		- thus each callback should do it on their own. Examples:
			- remover of container shall reinfer all its children items. 
			- remover of rigid body shall reinfer all its children fixtures and joints. 
			- remover of custom rigid body owner shall reinfer its body cache. its children fixtures. 
	- offload work to cache classes where it is reasonable
		- doesnt make sense for children vector trackers as they are in class of their own
	- pass inferred access keys to adders and removers
	- handle calls it either if the member function is present or if component is synchronized
		- so required in the latter case

<!--
Commented out because we've ditched the concept of activationness
	- Therefore, if a field at any time requires an inferred cache, there shall exist "default value" that does not require an inferred cache.
	- A sensitive field with default value cannot lie.
-->

	~~- If cache is constructed, **it is guaranteed to be consistent**, therefore return.~~
		- **Since we're going to allow incremental inferences of cache, it should always check if the cache is consistent and incrementally update it.**
			- And if it does not exist at all, probably use a fast function.
		- That is because if it is ever incrementally updated, it is done controllably.
		- When we know that this cache is no longer consistent due to being dependent on other cache, we always destroy it before calling inference.


	- The only moment that an entity is reinferred completely is when it changes a type.
		- Removing all components should imply complete destruction of inferred state.
			- Note: for the best intuition, there relation between caches and synchronized components shall be 1:1.
				- In particular, the relational cache shall be split into several classes.
					- There is always a component that implies parenthood, and component that implies childhood.
				- In particular, the physics world cache shall be split into several classes. 
					- And the class that holds b2World itself, which the caches share, shall do nothing on inference/destruction of entities
				- Additionally, parent and children trackers may want to be held separate?
					- Why do we need caches for children in the first place?
						- Alternative naming: parent_of_fixtures cache, child_of_body_cache?
							- child_of_body_cache needs not store data, only provide a callback
							- is_parent_set can just iterate through the parent cache while knowing the parent from significant
							- significant will anyways only ever be altered with safe functions
<!-- 
		- If cache depends on data from a single instance of a component, it shall trivially belong to one entity
-->
<!--
			- We *might*, for example, say that the children caches of one sort are dependent on other kind of cache of entity that they specify as their parent via some significant field.
				- Because their pointing to this entity as their parent implies some truths about caches.
-->
#### Notes

Note that now on_add and on_remove become obsolete because helper methods can decide on their own what should be reinferred.

- T: ~~changing an entity's type or thereof content and reinfering the whole cosmos right away shall never break consistency~~
	- WRONG: less space could've been specified for a container definition.
	- this is because associated fields will not be settable for definition's initial values; the associated fields will then be set to **defaults** which will always lead to reinference success
	- and we may require the solver to uphold the rule of never changing the type of any entity
		- theoretically we could let the solver change the type but it would be one more place to screw things up
		- it makes little practical difference for the author, actually none except that identity is broken but identity issues should be handled by the solver
			- basically the only place where we need it is the fuse?
- Therefore, we don't have to worry about changed types
	- However remember that complete reinference should infer identity and that which is inferred by the type; so it should do somewhat the same thing that explicit creation and adding components would do
- an entity's identity may also have a cache
	- all possible mutables/fluxing input to the simulation shall be determined at definition and creation stage
	- let an entity's type imply existence of some caches
	- solver shall not mutate component existence
- note that author can also mutate variables of invariants which would theoretically need reinference
	- currently we say that we reinfer the whole cosmos so not a problem and associateds will be private 
- existence of components is directly implied by invariants
	- if anything is implied by existence of components, it is then implied by invariants
	- dependencies
		- any function that changes associated state is on its own responsible for upholding consistency
- Make damping, density and owner bodies always possible to be calculated from the significant state.
	- damping/density values are only default data in rigid body
	- custom body owner is just an override
- Calculate activated field on the go, like damping and density. An author has completely no reason to specify a deactivated body from the start, and activationness should otherwise be calculated from the circumstances, e.g. from belonging to an item deposit.  
- separate "relational cache" into several classes
	- introduce relational_cache_mixin that provides reservation callback, set_relation and and stores the tracker.
	- concerns, are, I guess, separate: mapping of list of children is separate from mapping of body pointers
	- rigid body on_remove calls delete potential parents from joints of bodies and fixtures of bodies
	- container component on remove calls delete potential parent from items of slots
- determine EXACTLY dependencies between caches (or just for now make them in proper order and reinfer all on adding a associated component)
	- always refer to a proper visual graph of state dependencies
	- e.g. fixtures of bodies would depend on items of slots
	- each cache defines ``using depends_on = type_list<items_of_bodies, ...>`` etc.
		- on inference or destruction, iterate through all caches and reinfer them if they depend on the currently modified one  
- use augs::introspect to reserve entities on all caches and later optionally reinfer all caches 
- get_owner_of_colliders should return value from the inferred cache as technically an entity should always be inferred
	- the inferred cache should provide actual calculation from circumstances
- separate a "custom_rigid_body_owner" component so that caches/components have 1:1 ratio
	- so that fixtures cache does not need to construct both the relational and fixtural caches
- make operator= for synchronized components verify if there is a need of complete reinference, as just a single field could have changed

Assumptions that are the most relaxing:
- The author can only alter the first step of the simulation. 
	- Then they can as well be forbidden from possibly changing the type as we could step back and create entity with a different type in the first place. 
	- The editor data then becomes just a recipe for creating the simulation.
	- Then we can assume that the type of the entity stays the same throughout its lifetime.
		- What does it give?


- ~~associated significant with a counterpart in significant~~
	- should not exist, except in extreme circumstances where inference is so expensive, or if the code is not yet refactored
- associated significant with a counterpart in inferred...
	- ...whose any value in the significant ends in successful reinference.
	- ...for whom exist some values in the significant whose inference may fail
		- called **sensitive**.


#### Consistency upholders

These are the only functions that ever actually touch the associated (and thus sensitive) fields when the state is already consistent.  
They are callable from the cosmic functions and it is their requirement to uphold consistency of state.  
- ~~Change type of an entity~~. We will forbid this for simplicity. Later we might just facilitate this in the editor somehow.
	- It would be dangerous to change type of an existing entity without invalidating its identity.
		- That is because, if it was a container, and it changed to a container with less space, and its identity cache is left intact (e.g. its parenthood cache wasn't destroyed), then the state becomes inconsistent.
	- Destroys all caches of the existent entity, ~~except identity cache (e.g. does not destroy parenthood)~~ (poses a risk of breaking consistency).
<!--- T: Creating, cloning or destroying an entity does never break consistency.
- T: Changing an entity's type may possibly breaking
-->

- Examples of conduct with associated fields:
	- At **no point in time** can an entity have destroyed caches or only partially inferred, except as a part of reinference cycle.
	- Existence of components might be part of associated state itself.
		- but only for processing categories.
<!--
	- **Hide** all significant associated fields (no inferred state will be editable anyway) from the author, so that they only ever influence it through specialized methods that move them from one consistent state to the next.
		- Example: though not at all intuitive at a first glance, the **item component** (as opposed to the **definition**) should be completely immutable to the author. It should not be able to be deleted or added on demand.
			- Only expose a predictable ``perform_transfer`` function.
		- Although, obviously, in case of physics (velocity and others) an author might just see a slider like any else.
-->
- Provide a safe function that deletes all entity's components while preserving its identity.
	- Just call **remove** on all dynamic components and set always_present components **to default values**. **(except guid)**
	- Implies complete destruction of caches for an entity.
	- It is, however, required to move from one **consistent state to the next**.
		- If all associated state would only be associated by virtue of having an inferred cache...
			-  ...this would be as simple as destroying all caches related to that component.
		- Otherwise...
			- ...let components have "destructors". 
			- We'll probably go with this first for associateds that will take a lot of effort to move to the associated state.

- ~~with the current architecture, get_fixture_entities will only be needed in resolve_density_of_associated_fixtures; as dependencies will be well managed during inferences, the rigid body inferrer won't need to query the children~~
	- ~~thus we will be able to do away with fixtures_of_bodies cache?~~
	- Notice that when an item is hidden in deposit, then even though the body cache might be considered inferred, the b2Body will not exist for performance. Then when the current slot for this hidden item changes, it will need to iterate the children fixtures to reinfer them too as their caches are order-dependent on the body.

#### Detail cache functions

Domains in direct need of creating cache:
- Creating entity (whether by a clone, a delta or a genuinely new entity)
	- Should always easily succeed, problem starts with incremental alteration
- Complete reinference of the cosmos
- Methods for incremental alteration?
	- What is the largest alteration possible at a time?

Domains in direct need of destruction of some cache:
- Deleting entity
- Methods for incremental alteration?
- ~~Complete reinference of the cosmos~~
	- Complete reinference will just use class destructors.

- **Domains where inference and destruction of caches could happen arbitrarily:**
	- Complete reinference of the cosmos
	- Methods that change sensitive state while upholding consistency may need to arbitrarily reinfer some caches.
	- **This is especially important implication when dealing with parent/child relationships.**

- Component synchronizer's method that alters sensitive state and updates the caches to uphold consistency.
	- Calls responsible inferrer that will update incrementally.
	- If the associated field is sensitive
		- Catch [cosmos inconsistent error](cosmos_inconsistent_error) in case of failure.
			- Now the cache is destroyed.
			- In this case, restore old value and reinfer again with guarantee of success.

- What if complete reinference could be implemented in terms of entity creations/deletions?
	- Corner cases could theoretically be mitigated by enforcing order of construction?	

## entity flavour TRASH

<!--
- Entity can change the type during its lifetime, in which case it should be completely reinferred.
	- That will be helpful when we want to make a change to entity (that is impossible by just changing the components) while preserving correctness of all identificators that point to it.
	- Example: a grenade that changes from a normal body to a bullet body on being thrown (its sprite might change as well).
		- Most ``child_entity_id`` types will anyway be replaced with a entity_flavour_id. For other cases, it makes little sense to also enforce deletion of those entities just because they are meant to be related.
-->

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->


<!--
- ~~Entity should be reinferred if it changes a type (during content creation).~~ Type id for an entity should stay constant until its death.
	- If at all, changing the type should be implemented as an editor feature. Let us not make this an actual feature in the game code and let us always assume that an entity's type stays constant throughout its lifetime.
	- It will be easier in the beginning if we disallow this and require the author to just create a new entity.
-->
<!--
### Components considerations
An author should not be concerned with adding components to a new entity, that properly correspond to what they've already set in the invariants.  
Regardless of the fact that the invariants are statically allocated (and thus always "present"), the author should be able to specify which invariants they are interested in.  

Additionally, the author might want to specify initial values for the component that is added, that corresponds to the given definition type.
For example, some physical bodies might want to have some particular initial velocity set.  

However, the logic should, for the sake of performance and simplicity of code, always assume that a correspondent definition is present, to avoid noise.  
Thus the logic should derive usage of invariants from the presence of components.  
However, the presence of components will be initially derived from the presence of invariants.
It will thus be always required to define at least an empty type that specifies the correspondent component type,  
event if the component type itself does not need any definition-like data.

A concern could be raised because with this design, performance of serializing types could suffer.  
That is because, if we serialize linearly without regard for which invariants are set, we serialize possibly a lot of unneeded data.  
On the other hand, if we do serialize only that which was set, we may suffer from instruction cache misses as the code will not degrade to a simple memcpy anymore.

However, the type information:
	- Will not at all be that large;
	- Will not at all be copied around frequently (pretty much never, just the cosmos) 
	- Should have the topmost access performance, which would suffer from facilitating the structure for better serialization performance.

-->

<!--
### Performance

Some concern could be falsely raised to the fact that an entity's type id first need to be obtained, and only later the type information;  
as, compared to just a single component lookup, this could potentially be slower.

However, at the time of having an entity handle, the type id is already available, possibly in cache.  
The only thing left is to perform the lookup in the type information map.
Then, the only thing to compare performance-wise is a lookup in an ``unordered_map`` vs a [``pool``](pool).
If the container was optimized for deterministic identificators, it could possibly be faster;
Note that types won't be altered during logic, usually only during the creation of content.  
So, they could be stored in a fixed-sized container. **There can even be a vast performance improvement**. 
-->

<!--
		- if a definition implies more than one component, perhaps they should be merged?
		- That ensures that each component has a corresponding definition.
		- missile will imply both missile and sender?
			- we'll just add the sender component where necessary and where it wasn't yet added.
			- the sender, child components will anyway be hidden from the author as they are detail.~~
-->


<!--
	- Mutable or not, by the solver.
		- Preferably not, but it might be useful when partitioning polygons into destructed pieces.
	- They would take precedence over what would possibly be found in the correspondent invariants in the type.
	- When we will need to copy the game world, how do we determine if a shape is owned by an override or a pre-inferred definition? 
		- we need to take proper measures so as to never copy the shapes that are indeed flyweights; as opposed to the custom shapes.
	- The problem will not be frequent besides names and polygons, so it is a viable solution.
	- **Solution**: for each such case, create a dynamic component that, on its own, will not consume memory when not used, but can be named for example "custom polygon shape"
		- additionally, the dynamic component might have different assumptions about storage, since it will not be so common; it could even have plain ``std::vector``s instead of fixed-size containers.
			- although I would not advocate this.
		- Least memory wasted.
		- Best performance as the override checks will be limited to several specific domains.
		- Medium-hard conceptually.
	## entity flavour TRAS		- Additionally, it will be pretty much known from the get-go which data is possibly altered.
			- Nobody will also be surprised if the solver needs to alter that state, because they are plain components.
		- Good flexibility, until something unexpected needs to change.
	- **Solution**: create overrides for invariants and let an entity have its own set of invariants, if so is required.
		- Most memory wasted.
			- An entity with an override costs the size of all the invariants, unless we make each definition dynamically allocated, hindering overridden-entities performance all the more.
			- Additionally, as the type is the same, we must assume the same storage rules as with the invariants. In particular, we won't be able to use a dynamically allocated vector of vertices if the definition object already uses a constant size vector.
		- Worst performance as upon getting any definition we check for an existing override.
			- The only impact is on non-overridden entities.
			- Overidden entities would otherwise still need to get a component.
			- The impact might not be so great for the non-overriden entities as we might make the definiton id to already reside in cache once the type id is in cache.
		- Hardest conceptually.
			- Is it predictable when modifying any definition field? Some state might need to be updated to be consistent.
			- For example, rigid body parameters like type (static or dynamic, if it is a bullet etc.).
				- If we anyway allow for changing a type, then reinference should be enough.
		- Most flexibility in altering the game's behaviour.
	- The memory waste calculations for these two solutions actually assume that there will be a lot of overridden entities. 
		- If there are just a few, than having a dynamic component id for each possible override actually costs more in terms of aggregate size.
		- If there are many, then yes, per-entity overrides are worse. 
		- The actual serialized size should stay pretty much the same.
-->

<!--
### Simple per-field overrides

Consider this: usually, we specify damping values once for a given type of physical entity.  
However, the logic itself may want to alter the damping value due to some special circumstances (being inert, riding a motorcycle, sprinting, etc.).
Two approaches can be taken:

- ``std::optional<field_type> overridden_field;``
- ``field_type field_scalar;``

We'll probably use scalars most of the time.

COMMENTED OUT: Such things will be calculated statelessly.
-->


<!-- **We should disregard tailoring the assignment operator until we get to networking, where we'll probably switch to another physics engine with the features we need.** -->

## entity_flavour.md trash

	- An author may specify which invariants to enable.
	- The flags will be held separate:``std::array<bool, INVARIANT_COUNT_V> enabled_invariants;``
		- A lot better cache coherency than the more idiomatic ``std::optional``.

	- In particular, the **enabled** flags for invariants may change even though some entities of this type already exist.
		- We warn the author and ask if the existent entities should be recreated with new components with new initial values (preserving what was already set).
			- The warning can be ticked to never pop up again.

## other trash

- Rename hand_fuse to explosive_fuse and remove fuse timing from missile component;
	- invariant should have a boolean of whether it is unpinnable by hand
	- let also hand fuse system detonate explosive missiles

- position copying becomes the only way to change the entity's logic transform beside the rigid body and the wandering pixels
	- thus the only one with the problem of properly updating NPO 
		- We don't need NPO to track particle existences for now as there really won't be that many. 200 static ones would be so, so much already.
	- we can make it so that static particle existences don't even have the position copying component
- the only current use: particles and sound existences that chase after the entity
	- **solution**: 
	store a relevant field in each component that potentially needs it, 
	statelessly calculate transform when needed in particles simulation system,
	**statelessly determine whether particles should be activated (due to car engine or others) in particles simulation system** (that info could be stored in child component of chased entity)
	(not just activationness, the amount itself could be calculated)
		- pro: **quality of statelessness**
			- simplicity of code
			- less state transferred through the network
		- pro: particles and sounds can specify different chasings in the same entity
		- con: minimal duplication of code
		- con: worst performance as many WITH_PARTICLES_EXISTENCE need to be iterated through
			- can be optimized later via synchronizers and caches, without breaking content
			- as it's audiovisual mostly, this kind of thing can even be multithreaded
			- **and it moves load off the server**
			- if we're really furious, we can even make one thread continuously regenerate an npo from which then a renderer reads every frame
				- otherwise we would anyway need to update npo tree every frame which would be too big of a load for server
			- it can even be faster to iterate all particles existences while being cache-coherent.
	- additionally, expose a "get_particles_transform" to disambiguate get_logic_transform - the latter should not check for particles existence.
		- because a fixtural entity might additionally have a particles existence that chases upon itself.
			- e.g. truck engine
<!--
	- On prediction correction, we could:
		- disable all streams?
		- if each cosmos timeline held its copy of audiovisual caches... we could reassign along it.
		- In any case, logic is presumed to catch up quickly with sending messages for e.g. continuous streams 
			- because a cosmos might be at any time read from disk in which case it will have no audiovisual state

- Fire and forget streams should never last long.
- For continuous streams (e.g. decorations), we could make it so the logic is required to send a message once in a while so that the invalid caches "burn out" and the valid caches quickly start
	- Best if it only happens on startup and we keep copies of audiovisual caches for each cosmos and don't worry that they're not synchronized

- Particles are easy, can we pause all sounds if we want to keep a copy of audiovisual state for later revival?
	- Might anyway be implemented later.
-->

- Messages for particles and sounds are the best compromise because this state is then not synchronized and server might just opt out of processing messages itself with some compilation flag.
	- It's easier for audiovisual state not to iterate through all entities.
	- Startup catching up will not be that much of an issue
	- We can always do some "circumstantial" magic when overwriting cosmos signi because we have original input/start values saved
		- wouldn't anyways be perfect with particle existences, though at least you have times of births there
		- but storing audiovisual caches will provide the same flexibility

- Message intervals vs message every step
	- Message every step amortizes side-effects and would theoretically remove the need to store and reassign visual caches
		- as every time we can rebuild from messages
		- pro: don't have to come up with a sensible interval
			- no interval at all, actually, just persist caches through entire step and clear it on next post solve
		- pro: caches quickly catch up on any change
		- con: many messages posted 
			- but server can anyway opt out
		- con: has to clear audiovisual caches every time and prepare it anew? 
			- but ONLY for continuous streams
			- or just compare message contents to some last one so that it finds a suitable resource and determine whether some of it should be continued
	- Message per some interval
		- con: has to come up with nice interval for every effect 	
			- important only on startup
			- perhaps a stateless method that determines initial state of continuous streams?
				- decorations would have their own existential component with particle effect id
				- thus they would be somewhat similar to particle existences
		- con: has to cache audiovisual state for every timeline to avoid glitches
- PROBLEM: entity-chasing sound/particle streams might start chasing wrong entities if a cosmos gets reassigned from prediction correction!
	- Streams will either be very, very short in which case it won't matter (and it would suck to replay a sfx or move it in time randomly)
	- Or they will be continuous in which case they will be statelessly calculated.
		- Car engines and decorations.
- Discriminate sound/particle effect types and treat them accordingly. Might turn out that some caches might need to be copied, some not.
	- Think of explosions and how unpredicted explosions shall be handled, because it is important for the future.
		- Just played from the beginning, I believe.
	- Actually, no caches will need to be copied around.
	- Very short streams of sound or particles (e.g. sound on damage, but also its particle stream effect!) should be treated exactly like fire-and-forgets, despite the name.
	- E.g. it might make sense to make continuous stream of messages for continuous streams or enablable streams, because it is important for logic to catch up on them quickly.
	- Continuous streams in case of sound system might be music, for example.
		- Car engine and gun engine sounds as well!
	- Continuous streams might even be handled completely without messages, but statelessly.
	- So don't worry, we'll reuse what was written and add new things later for continuous objects. 
	- You've actually written about those stateless things...

- Solutions for "children entities": entities that are cloned with some other entity and deleted with it
	- **Chosen solution:** delete_with component that is a synchronized component
		- Most flexibility and separation of concerns; childhood is not really something that will change rapidly, as opposed to damping or density
		- Groups could then easily specify initial values for entities in the group
			- will just be a common practice to set the delete_with to the root member of the group
				- Currently would anyway be only used for crosshair
		- Concern: fast bi-directional access to:
			- parent having just child, 
				- easy: access delete_with component
			- or to child having just parent
				- has to access the children vector cache
				- but what if it has more children with varying functionality?
					- then we can add an enum to the child
				- do we say, we always treat the first child as e.g. a crosshair?
					- NO! We can simply associate a character with any entity as its crosshair!
					- The existential_child itself will only be used for deletion.
					- It may coincidentally point at its parent.
			- The difference is like this:
				- The parent can have many children, thus it makes no sense to make assumptions about order of children in the cache or some enums.
				- We'll just have entity_id's scattered through relevant domains that imply some acting upon the other entity or reading therefrom. 
				- If we have a delete_with component, we may assume some other role of its parent. 
					- **We should actually just make the child be associated coincidentally with just some entity_id**.
		- Concern: childhood type? 
			- Assumption: a child can only have one delete_with parent
				- But it may have more logical parents, e.g. it can be attached to some body as an item?
			- Do we want to assume that, if a child has delete_with set, that the associated parent fulfills some other role as well?
				- Concerns would mix, but that would imply less data which is fine
					- And that lets the rest of data stay raw and not synchronized
				- Rename to "logical_parent"?

## Needing confirmation:

- parenthood caches design:  
Current design with relational caches is correct.  
Parenthood requested by current values in signi should be tracked even if it is determined that, for a while, the actual functionality of the parent should be disabled (e.g. rigid body hidden in a backpack).  
Parenthood can be invalidated only with death of the parent itself, as even though only correct parents can be set in the first place,  
it is irrelevant for the relational cache whether the parents pointed to by ids do indeed fulfill parenthood conditions.  
Since possibility of parenthood is implied at definition stage, it makes sense to destroy that cache with death of the entity.  
	- Other than a memory optimization, we don't at all need to remove caches for a parent once it gets destroyed.  
Once children get destroyed, the cache will be freed automatically.
Additionally, the children will anyway be destroyed.
When we do "get_children_of" in a relational mixin, we can ensure that the entity is alive.
Memory is somewhat safe because it can only grow as far as the children grow.
		- Concern could be raised becasue that would mean that, after reinference, that cache would be drastically different. However, we make no guarantee of 0% reinference error. Functionally, the parent cache with dead parent id is equal to no cache. The code, however, will be simpler.
			- We will save that correction for later though.

### Add a component

- Stores the component index.
- Removes the component on undo.
    - This must obviously take proper measures to update processing lists and whatnot.

### Remove a component

- Stores the component index and the byte content.
- Adds the component on undo with the previous byte content.

	- how do we implement simple testbed fill?
		- overall support for other paths
		- %GAME_CONTENT% variable
			- if it is going to resolve to "content" then we might just as well write "content"
			- we're anyway not going to specify full paths and we want the map to be able to be read anywhere
			- so we must have a prioritization scheme, we first look in game's pwd, later in the map's dir
- proposed solution: .int + .tab + .autosave
	- con: a lot of files, **but that's not really a con**
		- because we will anyway have the image and sound files separate
			- because we want to advocate customizability and own versions
	- .autosave is guaranteed to always have both
	- .tab has history etc
	- later on saving perhaps more files could come into existence, like ".ruleset"
		- for plots and/or arenas
	- manual saving updates all other files and then purges autosave
	- we assume the editor is supposed to only ever work on int files as subjects
		- so we avoid some shenaningans with importing and exporting ints, it will just be for lua
- Filetype design
	- Saving should also write the history to disk.
	- It would be best if the unsaved file were either separate or just be one, maybe called "autosave".
		- Notice: since on autosave, we're writing all files, we could as well hold a single file.
			- Could just be a single blob held somewhere in cache.
			- Most of the time we will anyways edit just a single world.
			- We could name it "session".
	- Solution: separate .tab, .int and .history files 
		- Con: more checks for file existence
		- Con: a lot of files on disk
		- Pro? easier design on the side of code? possibly better separation and correspondence of structs and files
		- Pro: on saving, everything immediately relevant is already exported
		- Editor could create a folder for files
			- check how VS did it, maybe we could likewise use ranger?
			- it is nice though to have safety for extensions while writing, e.g. only allow particular extension as output
				- prevents accidents
			- I guess only intercosm files are important anyway.
				- We should also only allow lua imports and exports, without working explicitly on them.
		- Maybe we should ditch unsaved works and always save because it will only complicate things otherwise
			- Even Vim doesn't have the concept of untitled files
			- let alone a heavy editor like this should
	- Solution: .tab and .tab.unsaved
		- Pro: just one file for whole workspace, export specific things for the public
		- .tab is always binary
		- also contains binary history
		- can then export to .int or .lua
		
	- ``rules/``
		- ``rules/my.deathmatch`` - can be created and modified in editor, contains your own rules for a deathmatch, e.g. spawn points or identificators of flavours of guns that are available to each team, their costs as well
		- ``rules/duel.deathmatch`` - same as above but if you want to have specific spawnpoints for 1v1 you could define a separate file
		- ``rules/two_bombs.bombdefuse`` - setup for a bomb defusal game
		- ``rules/local.test`` with locally viewed entity for viewing in testbed and in the editor itself.
			- We could just have an entity_id locally viewed in the intercosm but it will be nice to start testing rulesets from such a simple example.
			- Editor will ask if a test ruleset should be created for viewing or always do so automatically when selecting a controlled entity.
				- It might automatically adjust other rulesets, or just the one currently selected.
