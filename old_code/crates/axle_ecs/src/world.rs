use eyre::Result;
use std::any::Any;

use crate::entities::{query::Query, Entities};
use crate::prelude::*;
use crate::resources::Resources;
use crate::systems::Systems;

/// The central hub of the ECS that stores all entities, systems, and resources.
#[derive(Default)]
pub struct World {
    /// The resources of the world. Resources are shared data that can be accessed by all systems.
    pub resources: Resources,
    /// The entities of the world. Entities are objects that can have components attached to them.
    pub entities: Entities,
    /// The systems of the world. Systems are functions that can access the entities and resources.
    pub systems: Systems,
}

impl World {
    /// Create a new, empty `World`.
    /// This funtion is the equivalent of calling `World::default()`
    pub fn new() -> Self {
        Self::default()
    }

    /// Add a resource to the world so that anyone with access to the world can query it.
    ///
    /// Resources are stored based on their type id.
    ///
    /// # Arguments
    ///
    /// * `resource_data` - The data you want to save as a resource.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    /// ```
    pub fn add_resource(&mut self, resource_data: impl Any) {
        self.resources.add(resource_data);
    }

    /// Query for a resource and get an immutable reference to it.
    ///
    /// The type of the resource must be added in so that it can be found.
    ///
    /// # Returns
    ///
    /// An option containing a reference to the resource.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    ///
    /// let resource = world.get_resource::<i32>().unwrap();
    ///
    /// assert_eq!(*resource, 10);
    /// ```
    pub fn get_resource<T: Any>(&self) -> Option<&T> {
        self.resources.get_ref::<T>()
    }

    /// Query for a resource and get a mutable reference to it.
    ///
    /// The type of the resource must be added in so that it can be found.
    ///
    /// # Returns
    ///
    /// An option containing a mutable reference to the resource.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    ///
    /// {
    ///     let resource = world.get_resource_mut::<i32>().unwrap();
    ///     *resource += 1;
    /// }
    ///
    /// let resource = world.get_resource::<i32>().unwrap();
    ///
    /// assert_eq!(*resource, 11);
    /// ```
    pub fn get_resource_mut<T: Any>(&mut self) -> Option<&mut T> {
        self.resources.get_mut::<T>()
    }

    /// This will remove the resource from the world.
    /// If the resource doesn't exist, it won't happen anything.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.add_resource(10_i32);
    ///
    /// world.delete_resource::<i32>();
    ///
    /// assert!(world.get_resource::<i32>().is_none());
    /// ```
    pub fn delete_resource<T: Any>(&mut self) {
        self.resources.remove::<T>();
    }

    /// This will register a component type to the world.
    /// Which will make it available for its use in entities.
    ///
    /// If you don't register a component, you won't be able to use it in entities.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.register_component::<i32>(); // Now you can use the i32 component in entities
    /// ```
    pub fn register_component<T: Any + 'static>(&mut self) {
        self.entities.register_component::<T>();
    }

    /// Create a new entity and return a mutable reference to the `Entities` struct
    /// so that you can add components to it.
    ///
    /// # Returns
    ///
    /// A mutable reference to the Entities struct.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.register_component::<u32>();
    ///
    /// world
    ///     .create_entity()
    ///     .with_component(100_u32).unwrap();
    /// ```
    pub fn create_entity(&mut self) -> &mut Entities {
        self.entities.create_entity()
    }

    /// Query the entities in the world.
    ///
    /// # Returns
    ///
    /// A new `Query` struct which you can use to query the entities in the world.
    ///
    /// # Example
    ///
    /// ```
    /// use std::{any::Any, cell::RefCell, rc::Rc};
    /// use axle_ecs::World;
    /// use axle_ecs::entities::query::{QueryIndexes, QueryComponents};
    ///
    /// let mut world = World::new();
    /// world.register_component::<u32>();
    /// world
    ///     .create_entity()
    ///     .with_component(100_u32).unwrap();
    ///
    /// let query: (QueryIndexes, QueryComponents) = world
    ///     .query()
    ///     .with_component::<u32>().unwrap()
    ///     .run();
    /// let u32s: &Vec<Rc<RefCell<dyn Any>>> = &query.1[0];
    ///
    /// assert_eq!(u32s.len(), 1);
    ///
    /// let borrowed_first_u32 = u32s[0].borrow();
    /// let first_u32 = borrowed_first_u32.downcast_ref::<u32>().unwrap();
    ///
    /// assert_eq!(*first_u32, 100_u32);
    /// ```
    pub fn query(&self) -> Query {
        Query::new(&self.entities)
    }

    /// Deletes the component from the entity with the given index.
    ///
    /// # Arguments
    ///
    /// * `index` - The index or id of the entity.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    /// use axle_ecs::entities::query::{QueryIndexes, QueryComponents};
    ///
    /// let mut world = World::new();
    /// world.register_component::<u32>();
    /// world.register_component::<i32>();
    /// world
    ///     .create_entity()
    ///     .with_component(100_u32).unwrap()
    ///     .with_component(100_i32).unwrap();
    /// world
    ///     .create_entity()
    ///     .with_component(200_u32).unwrap()
    ///     .with_component(200_i32).unwrap();
    ///
    /// world.delete_component_by_entity_id::<u32>(0).unwrap();
    ///
    ///  let query: (QueryIndexes, QueryComponents) = world.query()
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap()
    ///     .run();
    ///
    /// assert_eq!(query.0.len(), 1);
    /// assert_eq!(query.0[0], 1);
    ///
    /// ```
    pub fn delete_component_by_entity_id<T: Any>(&mut self, index: usize) -> Result<()> {
        self.entities.delete_component_by_entity_id::<T>(index)
    }

    /// Adds a component to an entity by the given id.
    ///
    /// # Arguments
    ///
    /// * `data` - The component's data.
    /// * `index` - The index or id of the entity.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.register_component::<u32>();
    /// world.register_component::<i32>();
    ///
    /// world
    ///     .create_entity()
    ///     .with_component(10_u32).unwrap();
    ///
    /// world.add_component_to_entity_by_id(10_i32, 0).unwrap();
    ///
    /// let query = world
    ///     .query()
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap()
    ///     .run();
    ///
    /// assert_eq!(query.0.len(), 1);
    /// assert_eq!(query.1.len(), 2);
    /// ```
    pub fn add_component_to_entity_by_id(&mut self, data: impl Any, index: usize) -> Result<()> {
        self.entities.add_component_by_entity_id(data, index)
    }

    /// Deletes an entity by its id.
    ///
    /// # Arguments
    ///
    /// * `index` - The id of the entity to delete.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::World;
    ///
    /// let mut world = World::new();
    /// world.register_component::<u32>();
    ///
    /// world
    ///     .create_entity()
    ///     .with_component(100_u32).unwrap();
    /// world
    ///     .create_entity()
    ///     .with_component(10_u32).unwrap();
    ///
    /// world.delete_entity_by_id(0).unwrap();
    ///
    /// let query = world
    ///     .query()
    ///     .with_component::<u32>().unwrap()
    ///     .run();
    ///
    /// assert_eq!(query.0.len(), 1);
    /// ```
    pub fn delete_entity_by_id(&mut self, index: usize) -> Result<()> {
        self.entities.delete_entity_by_id(index)
    }

    /// Creates a new system and adds it to the world.
    ///
    /// # Arguments
    ///
    /// * `system` - The system function to add.
    ///
    /// # Returns
    ///
    /// A mutable reference to the Systems struct.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::{entities::query_entity::QueryEntity, World};
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut world = World::new();
    ///
    /// world.register_component::<u32>();
    ///
    /// world.create_system(&|_: &Vec<QueryEntity>, _: &mut Resources| Ok(()));
    /// ```
    pub fn create_system(&mut self, system: SystemFunction) -> &mut Systems {
        self.systems.create_system(system)
    }

    /// Deletes a component from a system by its id.
    ///
    /// # Arguments
    ///
    /// * `index` - The id of the system to delete the component from.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::{entities::query_entity::QueryEntity, World};
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut world = World::new();
    /// world
    ///     .create_system(&|_: &Vec<QueryEntity>, _: &mut Resources| Ok(()))
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap();
    ///
    /// world.delete_component_by_system_id::<u32>(0).unwrap();
    /// ```
    pub fn delete_component_by_system_id<T: Any>(&mut self, index: usize) -> Result<()> {
        self.systems.delete_component_by_system_id::<T>(index)
    }

    /// Adds a component to a system by its id.
    ///
    /// # Arguments
    ///
    /// * `index` - The id of the system to add the component to.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::{entities::query_entity::QueryEntity, World};
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut world = World::new();
    ///
    /// world.create_system(&|_: &Vec<QueryEntity>, _: &mut Resources| Ok(()));
    ///
    /// world.add_component_to_system_by_id::<u32>(0).unwrap();
    /// world.add_component_to_system_by_id::<i32>(0).unwrap();
    /// ```
    pub fn add_component_to_system_by_id<T: Any>(&mut self, index: usize) -> Result<()> {
        self.systems.add_component_by_system_id::<T>(index)
    }

    /// Deletes a system by its id.
    ///
    /// # Arguments
    ///
    /// * `index` - The id of the system to delete.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::{entities::query_entity::QueryEntity, World};
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut world = World::new();
    ///
    /// world
    ///     .create_system(&|_: &Vec<QueryEntity>, _: &mut Resources| Ok(()))
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap();
    ///
    /// world.delete_system_by_id(0).unwrap();
    /// ```
    pub fn delete_system_by_id(&mut self, index: usize) -> Result<()> {
        self.systems.delete_system_by_id(index)
    }

    /// Runs all systems and updates the entities.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::{entities::query_entity::QueryEntity, World};
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut world = World::new();
    ///
    /// world.register_component::<u32>();
    /// world.register_component::<i32>();
    ///
    /// world
    ///     .create_system(&|_: &Vec<QueryEntity>, _: &mut Resources| Ok(()))
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap();
    ///
    /// world.run_all_systems().unwrap();
    ///
    /// ```
    pub fn run_all_systems(&mut self) -> Result<()> {
        self.systems.run_all(&self.entities, &mut self.resources)
    }
}
