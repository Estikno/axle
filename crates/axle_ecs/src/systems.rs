use eyre::Result;
use std::any::{Any, TypeId};

use crate::entities::{query::Query, Entities};
use crate::prelude::*;
use crate::resources::Resources;

pub type SystemFunction = &'static dyn Fn(&Vec<QueryEntity>) -> Result<()>;
pub type SystemComponents = Vec<TypeId>;

/// Stores all the systems in the ECS.
#[derive(Default)]
pub struct Systems {
    /// The system functions stored as a vector of options.
    funtions: Vec<Option<SystemFunction>>,
    /// The components each system has stored as a vector of vectors of type ids.
    components: Vec<SystemComponents>,
    /// The resources each system has, stored as a vector of vectors of type ids.
    resources: Vec<Vec<TypeId>>,
    /// The index of the system to add a component to.
    inserting_into_index: usize,
}

impl Systems {
    /// Creates a new system and adds it to the data.
    ///
    /// # Arguments
    ///
    /// * `system` - The system function to add.
    ///
    /// # Returns
    ///
    /// A mutable reference to this struct.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    ///
    /// let mut systems = Systems::default();
    /// systems.create_system(&|_: &Vec<QueryEntity>| Ok(()));
    /// ```
    pub fn create_system(&mut self, system: SystemFunction) -> &mut Self {
        if let Some(index) = self.funtions.iter().position(|x| x.is_none()) {
            self.inserting_into_index = index;
            self.funtions[index] = Some(system);
        } else {
            self.funtions.push(Some(system));
            self.components.push(vec![]);
            self.resources.push(vec![]);
            self.inserting_into_index = self.funtions.len() - 1;
        }

        self
    }

    /// Add a component to a creating system.
    /// The component will be added to the current system.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the component to add.
    ///
    /// # Returns
    ///
    /// A result containing a mutable reference to this struct.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    ///
    /// let mut systems = Systems::default();
    /// systems
    ///     .create_system(&|_: &Vec<QueryEntity>| Ok(()))
    ///     .with_component::<u32>().unwrap()
    ///     .with_component::<i32>().unwrap();
    /// ```
    pub fn with_component<T: Any>(&mut self) -> Result<&mut Self> {
        let type_id = TypeId::of::<T>();

        let components = self
            .components
            .get_mut(self.inserting_into_index)
            .ok_or(CustomErrors::CreateSystemNeverCalled)?;
        components.push(type_id);

        Ok(self)
    }

    pub fn with_resource<T: Any>(&mut self) -> Result<&mut Self> {
        let type_id = TypeId::of::<T>();

        let resources = self
            .resources
            .get_mut(self.inserting_into_index)
            .ok_or(CustomErrors::CreateSystemNeverCalled)?;
        resources.push(type_id);

        Ok(self)
    }

    /// Deletes a component from a system by the given system id and component type.
    ///
    /// # Arguments
    ///
    /// * `system_id` - The id of the system to delete the component from.
    /// * `T` - The type of the component to delete.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    ///
    /// let mut systems = Systems::default();
    /// systems
    ///     .create_system(&|_: &Vec<QueryEntity>| Ok(()))
    ///     .with_component::<i32>().unwrap()
    ///     .with_component::<u32>().unwrap();
    ///
    /// systems.delete_component_by_system_id::<i32>(0).unwrap();
    /// ```
    pub fn delete_component_by_system_id<T: Any>(&mut self, system_id: usize) -> Result<()> {
        let type_id = TypeId::of::<T>();
        let index = self.components[system_id]
            .iter()
            .position(|id| *id == type_id)
            .ok_or(CustomErrors::ComponentInSystemDoesNotExist)?;

        self.components[system_id].remove(index);

        Ok(())
    }

    pub fn delete_resource_by_system_id<T: Any>(&mut self, system_id: usize) -> Result<()> {
        let type_id = TypeId::of::<T>();
        let index = self.resources[system_id]
            .iter()
            .position(|id| *id == type_id)
            .ok_or(CustomErrors::ResourceInSystemDoesNotExist)?;

        self.resources[system_id].remove(index);

        Ok(())
    }

    /// Adds a component to a system by the given system id and component type.
    ///
    /// # Arguments
    ///
    /// * `system_id` - The id of the system to add the component to.
    /// * `T` - The type of the component to add.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    ///
    /// let mut systems = Systems::default();
    /// systems
    ///     .create_system(&|_: &Vec<QueryEntity>| Ok(()))
    ///     .with_component::<i32>().unwrap();
    ///
    /// systems.add_component_by_system_id::<u32>(0).unwrap();
    /// ```
    pub fn add_component_by_system_id<T: Any>(&mut self, system_id: usize) -> Result<()> {
        let type_id = TypeId::of::<T>();
        let components = self
            .components
            .get_mut(system_id)
            .ok_or(CustomErrors::SystemDoesNotExist)?;

        components.push(type_id);

        Ok(())
    }

    pub fn add_resource_by_system_id<T: Any>(&mut self, system_id: usize) -> Result<()> {
        let type_id = TypeId::of::<T>();
        let resources = self
            .resources
            .get_mut(system_id)
            .ok_or(CustomErrors::SystemDoesNotExist)?;

        resources.push(type_id);

        Ok(())
    }

    /// Deletes a system by its id.
    ///
    /// # Arguments
    ///
    /// * `system_id` - The id of the system to delete.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    ///
    /// let mut systems = Systems::default();
    /// systems
    ///     .create_system(&|_: &Vec<QueryEntity>| Ok(()))
    ///     .with_component::<i32>().unwrap();
    ///
    /// systems.delete_system_by_id(0).unwrap();
    /// ```
    pub fn delete_system_by_id(&mut self, system_id: usize) -> Result<()> {
        let function = self
            .funtions
            .get_mut(system_id)
            .ok_or(CustomErrors::SystemDoesNotExist)?;
        *function = None;

        // clears the inside components vector
        let components = self
            .components
            .get_mut(system_id)
            .ok_or(CustomErrors::SystemDoesNotExist)?;
        components.clear();

        // clears the inside resources vector
        let resources = self
            .resources
            .get_mut(system_id)
            .ok_or(CustomErrors::SystemDoesNotExist)?;
        resources.clear();

        Ok(())
    }

    /// Runs all the systems created.
    ///
    /// # Arguments
    ///
    /// * `entities` - A refence to the entities.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::entities::{query_entity::QueryEntity, Entities};
    /// use axle_ecs::systems::Systems;
    /// use axle_ecs::resources::Resources;
    ///
    /// let mut entities = Entities::default();
    /// let mut resources = Resources::default();
    ///
    /// entities.register_component::<u32>();
    /// entities.register_component::<i32>();
    ///
    /// let mut systems = Systems::default();
    /// systems
    ///     .create_system(&|_: &Vec<QueryEntity>| Ok(()))
    ///     .with_component::<i32>().unwrap();
    ///
    /// entities
    ///     .create_entity()
    ///     .with_component(100_i32).unwrap()
    ///     .with_component(100_u32).unwrap();
    ///
    /// systems.run_all(&entities, &resources).unwrap();
    /// ```
    pub fn run_all(&self, entities: &Entities, resources: &Resources) -> Result<()> {
        for index in 0..self.funtions.len() {
            if let Some(function) = self.funtions[index] {
                let mut query = Query::new(entities);

                let components = &self.components[index];
                components.iter().for_each(|type_id| {
                    query.with_component_by_type_id(*type_id).unwrap();
                });

                (function)(&query.run_entity())?;
            }
        }

        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::entities::{query_entity::QueryEntity, Entities};
    use crate::resources::Resources;

    #[test]
    fn create_system() {
        let mut systems = Systems::default();
        systems.create_system(&damage_health);

        assert_eq!(systems.funtions.len(), 1);
        assert_eq!(systems.components.len(), 1);
    }

    #[test]
    fn create_system_with_components() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?
            .with_component::<Speed>()?;

        assert_eq!(systems.funtions.len(), 1);
        assert_eq!(systems.components.len(), 1);
        assert_eq!(systems.components[0].len(), 2);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());
        assert_eq!(systems.components[0][1], TypeId::of::<Speed>());

        Ok(())
    }

    #[test]
    fn create_system_with_resources() -> Result<()> {
        let mut systems = Systems::default();

        systems
            .create_system(&damage_health)
            .with_component::<Health>()?
            .with_component::<Speed>()?
            .with_resource::<Score>()?;

        assert_eq!(systems.resources.len(), 1);
        assert_eq!(systems.resources[0].len(), 1);
        assert_eq!(systems.resources[0][0], TypeId::of::<Score>());

        Ok(())
    }

    #[test]
    fn excecute_system_on_entity() -> Result<()> {
        let mut entities = Entities::default();
        let resources = Resources::default();

        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(100))?;

        systems.run_all(&entities, &resources)?;

        let mut query = Query::new(&entities);
        let query_result = query.with_component::<Health>()?.run();

        let healths = &query_result.1[0];
        let wrapped_health = healths[0].borrow();
        let health = wrapped_health.downcast_ref::<Health>().unwrap();

        assert_eq!(health.0, 90_u32);

        Ok(())
    }

    #[test]
    fn excecute_multiples_systems_on_entities() -> Result<()> {
        let mut entities = Entities::default();
        let resources = Resources::default();

        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        systems
            .create_system(&increase_speed)
            .with_component::<Speed>()?;

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(0))?;

        entities.create_entity().with_component(Speed(10))?;

        entities.create_entity().with_component(Health(200))?;

        entities.create_entity().with_component(Health(300))?;

        systems.run_all(&entities, &resources)?;

        // See health stats
        let mut query = Query::new(&entities);
        let query_result = query.with_component::<Health>()?.run();

        let healths = &query_result.1[0];

        let wrapped_first_health = healths[0].borrow();
        let first_health = wrapped_first_health.downcast_ref::<Health>().unwrap();
        assert_eq!(first_health.0, 90_u32);

        let wrapped_second_health = healths[1].borrow();
        let second_health = wrapped_second_health.downcast_ref::<Health>().unwrap();
        assert_eq!(second_health.0, 190_u32);

        let wrapped_third_health = healths[2].borrow();
        let third_health = wrapped_third_health.downcast_ref::<Health>().unwrap();
        assert_eq!(third_health.0, 290_u32);

        // See speed stats
        let mut query = Query::new(&entities);
        let query_result = query.with_component::<Speed>()?.run();

        let speeds = &query_result.1[0];

        let wrapped_first_speed = speeds[0].borrow();
        let first_speed = wrapped_first_speed.downcast_ref::<Speed>().unwrap();
        assert_eq!(first_speed.0, 10_u32);

        let wrapped_second_speed = speeds[1].borrow();
        let second_speed = wrapped_second_speed.downcast_ref::<Speed>().unwrap();
        assert_eq!(second_speed.0, 20_u32);

        Ok(())
    }

    #[test]
    fn delete_component_by_system_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Speed>()?
            .with_component::<Health>()?;

        // delete components
        systems.delete_component_by_system_id::<Speed>(0)?;

        assert_eq!(systems.components[0].len(), 1);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());

        Ok(())
    }

    #[test]
    fn delete_resource_by_system_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?
            .with_resource::<Score>()?
            .with_resource::<Speed>()?;

        systems.delete_resource_by_system_id::<Score>(0)?;

        assert_eq!(systems.resources[0].len(), 1);
        assert_eq!(systems.resources[0][0], TypeId::of::<Speed>());

        Ok(())
    }

    #[test]
    fn add_component_by_entity_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        systems.add_component_by_system_id::<Speed>(0)?;

        assert_eq!(systems.components[0].len(), 2);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());
        assert_eq!(systems.components[0][1], TypeId::of::<Speed>());

        Ok(())
    }

    #[test]
    fn add_resource_by_entity_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?
            .with_resource::<Score>()?;

        systems.add_resource_by_system_id::<Speed>(0)?;

        assert_eq!(systems.resources[0].len(), 2);
        assert_eq!(systems.resources[0][0], TypeId::of::<Score>());
        assert_eq!(systems.resources[0][1], TypeId::of::<Speed>());

        Ok(())
    }

    #[test]
    fn delete_system_by_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        systems.delete_system_by_id(0)?;

        assert_eq!(systems.components.len(), 1);
        assert_eq!(systems.funtions.len(), 1);
        assert!(systems.funtions[0].is_none());

        Ok(())
    }

    #[test]
    fn created_systems_are_inserted_into_deleted_systems_columns() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        systems
            .create_system(&increase_speed)
            .with_component::<Speed>()?;

        systems.delete_system_by_id(0)?;

        systems
            .create_system(&both)
            .with_component::<Health>()?
            .with_component::<Speed>()?;

        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        assert_eq!(systems.funtions.len(), 3);
        assert_eq!(systems.components.len(), 3);

        assert!(systems.funtions[0].is_some());
        assert!(systems.funtions[1].is_some());
        assert!(systems.funtions[2].is_some());

        assert_eq!(systems.components[0].len(), 2);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());
        assert_eq!(systems.components[0][1], TypeId::of::<Speed>());

        Ok(())
    }

    #[test]
    fn should_happen_nothing_after_deleting_multiple_times_the_same_id() -> Result<()> {
        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        systems
            .create_system(&increase_speed)
            .with_component::<Speed>()?;

        systems.delete_system_by_id(0)?;
        systems.delete_system_by_id(0)?;
        systems.delete_system_by_id(0)?;

        systems
            .create_system(&both)
            .with_component::<Health>()?
            .with_component::<Speed>()?;

        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        assert_eq!(systems.funtions.len(), 3);
        assert_eq!(systems.components.len(), 3);

        assert!(systems.funtions[0].is_some());
        assert!(systems.funtions[1].is_some());
        assert!(systems.funtions[2].is_some());

        assert_eq!(systems.components[0].len(), 2);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());
        assert_eq!(systems.components[0][1], TypeId::of::<Speed>());

        Ok(())
    }

    fn damage_health(entities: &Vec<QueryEntity>) -> Result<()> {
        for entity in entities {
            let mut health_mut = entity.get_component_mut::<Health>()?;
            health_mut.0 -= 10;
        }

        Ok(())
    }

    fn increase_speed(entities: &Vec<QueryEntity>) -> Result<()> {
        for entity in entities {
            let mut speed_mut = entity.get_component_mut::<Speed>()?;
            speed_mut.0 += 10;
        }

        Ok(())
    }

    fn both(entities: &Vec<QueryEntity>) -> Result<()> {
        for entity in entities {
            let mut health_mut = entity.get_component_mut::<Health>()?;
            let mut speed_mut = entity.get_component_mut::<Speed>()?;

            health_mut.0 -= 10;
            speed_mut.0 += 10;
        }

        Ok(())
    }

    struct Health(pub u32);
    struct Speed(pub u32);
    struct Score(pub u32);
}
