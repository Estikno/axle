use std::{any::{Any, TypeId}, cell::RefCell, collections::HashMap, rc::Rc};
use eyre::Result;

use crate::prelude::*;

pub mod query;
pub mod query_entity;

pub type Component = Rc<RefCell<dyn Any + 'static>>;
pub type Components = HashMap<TypeId, Vec<Option<Component>>>;

/// The main struct for storing and managing entities and their components.
#[derive(Debug, Default)]
pub struct Entities {
    /// A hasmap containing the components for every entity.
    ///
    /// The type id is used to identify the type of the component, and the vector
    /// contains the actual components.
    /// 
    /// For example, to get the the component `Health` of the second entity, it will be like this: `components.get(&TypeId::of::<Health>())[1]`
    components: Components,

    /// The bitmasks of every component's registered type id
    ///
    /// The type id is used to identify the type of the component, and the bit
    /// mask is used to identify which components are enabled for a given entity.
    /// 
    /// The bitmsk of every component is a bit shifted an adition than the previous component.
    /// For example, the first component is `0001` and the second component is `0010`.
    bit_masks: HashMap<TypeId, u32>,

    /// A vector of bit masks for every entity.
    ///
    /// The bit mask is used to store which components are enabled for a given
    /// entity.
    /// 
    /// For example, if the entity has the first and third registered component, its map will be `101`.
    map: Vec<u32>,

    /// The index of the entity that is being inserted into.
    ///
    /// This is used to keep track of which entity is being inserted into when
    /// creating and inserting components in it.
    inserting_into_index: usize,
}

impl Entities {
    /// Registers a component for later use in entities.
    ///
    /// # Arguments
    /// 
    /// * `T` - The type of the component to register.
    pub fn register_component<T: Any>(&mut self) {
        let type_id = TypeId::of::<T>();

        self.components.insert(type_id, vec![]);
        self.bit_masks.insert(type_id, 1 << self.bit_masks.len());
    }

    /// Creates a new entity and returns a mutable reference to the `Entities` struct
    /// so that you can add components to it.
    ///
    /// # Returns
    ///
    /// A mutable reference to the Entities struct.
    pub fn create_entity(&mut self) -> &mut Self {
        if let Some((index, _)) = self.map.iter().enumerate().find(|(_index, mask)| **mask == 0) {
            self.inserting_into_index = index;
        }
        else {
            self.components
                .iter_mut()
                .for_each(|(_key, components)| components.push(None));

            self.map.push(0);
            self.inserting_into_index = self.map.len() - 1;
        }

        self
    }

    /// Adds a component to the current entity.
    ///
    /// # Arguments
    ///
    /// * `data` - The component's data.
    ///
    /// # Returns
    ///
    /// A result that contains a mutable reference to the `Entities` struct if succeeds or an error if it fails.
    ///
    /// # Errors
    ///
    /// If the component was not registered, or if the `create_entity` function was not called before,
    /// an error is returned.
    pub fn with_component(&mut self, data: impl Any) -> Result<&mut Self> {
        let type_id = data.type_id();
        let index = self.inserting_into_index;
        
        // Check if the component was registered and if the create entity function was called before
        if let Some(components) = self.components.get_mut(&type_id) {
            let component = components
                .get_mut(index)
                .ok_or(CustomErrors::CreateEntityNeverCalled)?;
            *component = Some(Rc::new(RefCell::new(data)));

            // Add the component's bitmask to the entity's bitmask
            let bit_mask = self.bit_masks.get(&type_id).unwrap();
            self.map[index] |= *bit_mask;
        }
        else {
            return Err(CustomErrors::ComponentNotRegistered.into());
        }

        Ok(self)
    }

    /// Gets the bitmask of a component type.
    ///
    /// # Arguments
    ///
    /// * `type_id` - The type id of the component type.
    ///
    /// # Returns
    ///
    /// The bitmask of the component type if it exists, otherwise `None`.
    pub fn get_bitmask(&self, type_id: &TypeId) -> Option<u32> {
        self.bit_masks.get(type_id).copied()
    }

    /// Gets the bitmask of an entity.
    ///
    /// # Arguments
    ///
    /// * `entity_id` - The id of the entity.
    ///
    /// # Returns
    ///
    /// The bitmask of the entity if it exists, otherwise `None`.
    pub fn get_map(&self, entity_id: usize) -> Option<u32> {
        self.map.get(entity_id).copied()
    }

    /// Deletes a component from an entity.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the component to delete.
    /// * `index` - The id of the entity to delete the component from.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    pub fn delete_component_by_entity_id<T: Any>(&mut self, index: usize) -> Result<()> {
        let type_id = TypeId::of::<T>();
        let mask = match self.bit_masks.get(&type_id) {
            Some(mask) => mask,
            None => return Err(CustomErrors::ComponentNotRegistered.into()),
        };

        // Check if the entity has the component
        if self.has_component(index, *mask) {
            // Remove the component's bitmask from the entity's bitmask
            self.map[index] ^= *mask;
        }

        Ok(())
    }

    /// Adds a component to an entity by the given id.
    ///
    /// # Arguments
    ///
    /// * `data` - The component's data.
    /// * `index` - The id of the entity to add the component to.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    ///
    /// # Errors
    ///
    /// If the component was not registered, or if the `create_entity` function was not called before,
    /// an error is returned.
    pub fn add_component_by_entity_id(&mut self, data: impl Any, index: usize) -> Result<()> {
        let type_id = data.type_id();
        let mask = match self.bit_masks.get(&type_id) {
            Some(mask) => mask,
            None => return Err(CustomErrors::ComponentNotRegistered.into()),
        };

        // Add the component's bitmask to the entity's bitmask
        self.map[index] |= *mask;

        let components = self.components.get_mut(&type_id).unwrap();
        // Add the component's data to the entity
        components[index] = Some(Rc::new(RefCell::new(data)));

        Ok(())
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
    /// # Errors
    ///
    /// If the entity doesn't exist, an error is returned.
    pub fn delete_entity_by_id(&mut self, index: usize) -> Result<()> {
        // Check if the entity exists
        if let Some(map) = self.map.get_mut(index) {
            // Reset the entity's bitmask, effectively deleting it
            *map = 0;
        }
        else {
            // If the entity doesn't exist, return an error
            return Err(CustomErrors::EntityDoesNotExist.into());
        }

        Ok(())
    }

    /// Checks if an entity has a component.
    ///
    /// # Arguments
    ///
    /// * `index` - The id of the entity to check.
    /// * `mask` - The bitmask of the component to check for.
    ///
    /// # Returns
    ///
    /// `true` if the entity has the component, `false` otherwise.
    fn has_component(&self, index: usize, mask: u32) -> bool {
        self.map[index] & mask == mask
    }
}

#[cfg(test)]
mod tests {
    use std::any::TypeId;

    use super::*;

    #[test]
    fn register_an_entity() {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        let type_id = TypeId::of::<Health>();

        let health_components = entities.components.get(&type_id).unwrap();

        assert_eq!(health_components.len(), 0);
    }

    #[test]
    fn bitmask_updated_when_registering_entities() {
        let mut entities = Entities::default();

        entities.register_component::<Health>();
        let type_id = TypeId::of::<Health>();
        let mask = entities.bit_masks.get(&type_id).unwrap();
        assert_eq!(*mask, 1);

        entities.register_component::<Speed>();
        let type_id = TypeId::of::<Speed>();
        let mask = entities.bit_masks.get(&type_id).unwrap();
        assert_eq!(*mask, 2);
    }

    #[test]
    fn create_entity() {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        entities.register_component::<Speed>();
        entities.create_entity();

        let health = entities.components.get(&TypeId::of::<Health>()).unwrap();
        let speed = entities.components.get(&TypeId::of::<Speed>()).unwrap();
        
        assert!(health.len() == speed.len() && health.len() == 1);
        assert!(health[0].is_none() && speed[0].is_none());
    }

    #[test]
    fn with_component() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        entities.register_component::<Speed>();
        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(15))?;

        let first_health = &entities.components.get(&TypeId::of::<Health>()).unwrap()[0];
        let wrapped_health = first_health.as_ref().unwrap();
        let borrowed_health = wrapped_health.borrow();
        let health = borrowed_health.downcast_ref::<Health>().unwrap();

        assert_eq!(health.0, 100);
        Ok(())
    }

    #[test]
    fn map_is_updated_when_creating_entities() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        entities.register_component::<Speed>();
        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(15))?;

        let entity_map = entities.map[0];
        assert_eq!(entity_map, 3);
        
        entities
            .create_entity()
            .with_component(Speed(15))?;

        let entity_map = entities.map[1];
        assert_eq!(entity_map, 2);

        Ok(())
    }

    #[test]
    fn delete_component_by_entity_id() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(50))?;

        entities.delete_component_by_entity_id::<Health>(0)?;

        assert_eq!(entities.map[0], 2);

        Ok(())
    }

    #[test]
    fn add_component_to_entity_by_id() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        entities
            .create_entity()
            .with_component(Health(100))?;

        entities.add_component_by_entity_id(Speed(50), 0)?;

        assert_eq!(entities.map[0], 3);

        let speed_type_id = TypeId::of::<Speed>();
        let wrapped_speeds = entities.components.get(&speed_type_id).unwrap();
        let wrapped_speed = wrapped_speeds[0].as_ref().unwrap();
        let borowed_speed = wrapped_speed.borrow();
        let speed = borowed_speed.downcast_ref::<Speed>().unwrap();

        assert_eq!(speed.0, 50);

        Ok(())
    }

    #[test]
    fn delete_entity_by_id() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();
        
        entities
            .create_entity()
            .with_component(Health(100))?;

        entities.delete_entity_by_id(0)?;

        assert_eq!(entities.map[0], 0);

        Ok(())
    }

    #[test]
    fn created_entities_are_inserted_into_deleted_entities_columns() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<Health>();

        entities
            .create_entity()
            .with_component(Health(100))?;
        entities
            .create_entity()
            .with_component(Health(50))?;

        entities.delete_entity_by_id(0)?;

        entities
            .create_entity()
            .with_component(Health(25))?;

        entities
            .create_entity()
            .with_component(Health(75))?;

        assert_eq!(entities.map[0], 1);

        let type_id = TypeId::of::<Health>();
        let healths = entities.components.get(&type_id).unwrap();
        let wrapped_health = &healths[0];
        let borrowed_health = wrapped_health.as_ref().unwrap().borrow();
        let health = borrowed_health.downcast_ref::<Health>().unwrap();

        assert_eq!(health.0, 25);

        let type_id = TypeId::of::<Health>();
        let healths = entities.components.get(&type_id).unwrap();
        let wrapped_health = &healths[2];
        let borrowed_health = wrapped_health.as_ref().unwrap().borrow();
        let health = borrowed_health.downcast_ref::<Health>().unwrap();

        assert_eq!(health.0, 75);
        
        Ok(())
    }

    #[test]
    fn should_not_add_component_back_after_deleting_twice() -> Result<()> {
        let mut entities = Entities::default();
        entities.register_component::<u32>();
        entities.register_component::<f32>();

        entities
            .create_entity()
            .with_component(100_u32)?
            .with_component(50.0_f32)?;

        entities.delete_component_by_entity_id::<u32>(0)?;
        entities.delete_component_by_entity_id::<u32>(0)?;

        assert_eq!(entities.map[0], 2);

        Ok(())
    }

    struct Health(pub u32);
    struct Speed(pub u32);
}