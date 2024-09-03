use std::{any::{Any, TypeId}, collections::HashMap};

/// A structure to store resources in the game world.
#[derive(Default)]
pub struct Resources {
    /// The hashmap of resources, where the key is the type id of the resource
    /// and the value is the actual resource.
    data: HashMap<TypeId, Box<dyn Any>>,
}

impl Resources {
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
    /// use axle_ecs::resources::Resources;
    /// 
    /// let mut resources = Resources::default();
    /// let world_width = 100.0_f32;
    /// 
    /// resources.add(world_width);
    /// ```
    pub fn add(&mut self, data: impl Any) {
        let type_id = data.type_id();
        self.data.insert(type_id, Box::new(data));
    }

    /// Get a reference to a resource.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the resource you want to get a reference to.
    ///
    /// # Returns
    ///
    /// An option containing a reference to the resource. If the resource doesn't exist, the option is `None`.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::resources::Resources;
    /// 
    /// let mut resources = Resources::default();
    /// let world_width = 100_f32;
    /// 
    /// resources.add(world_width);
    /// 
    /// let extracted_width: &f32 = resources.get_ref::<f32>().unwrap();
    /// ```
    pub fn get_ref<T: Any>(&self) -> Option<&T> {
        let type_id = TypeId::of::<T>();

        if let Some(data) = self.data.get(&type_id) {
            data.downcast_ref()
        }
        else {
            None
        }
    }

    /// Get a mutable reference to a resource.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the resource you want to get a mutable reference to.
    ///
    /// # Returns
    ///
    /// An option containing a mutable reference to the resource. If the resource doesn't exist, the option is `None`.
    ///
    /// # Example
    ///
    /// ```
    /// use axle_ecs::resources::Resources;
    /// 
    /// let mut resources = Resources::default();
    /// let world_width = 100.0_f32;
    /// 
    /// resources.add(world_width);
    /// 
    /// let world_width: &mut f32 = resources.get_mut::<f32>().unwrap();
    /// *world_width += 1.0;
    /// ```
    pub fn get_mut<T: Any>(&mut self) -> Option<&mut T> {
        let type_id = TypeId::of::<T>();

        if let Some(data) = self.data.get_mut(&type_id) {
            data.downcast_mut()
        }
        else {
            None
        }
    }

    /// Remove a resource from the `Resources` by its type.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the resource you want to get a mutable reference to.
    /// 
    /// # Example
    /// 
    /// ```
    /// use axle_ecs::resources::Resources;
    /// 
    /// let mut resources = Resources::default();
    /// let world_width = 100.0_f32;
    /// 
    /// resources.add(world_width);
    /// 
    /// resources.remove::<f32>();
    /// ```
    pub fn remove<T: Any>(&mut self) {
        let type_id = TypeId::of::<T>();
        self.data.remove(&type_id);
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn add_resource() {
        let resources = initialize_resource();

        let stored_resource = resources.data.get(&TypeId::of::<WorldWidth>()).unwrap();
        let extracted_world_width = stored_resource.downcast_ref::<WorldWidth>().unwrap();

        assert_eq!(extracted_world_width.0, 100.0);
    }

    #[test]
    fn get_resource() {
        let resources = initialize_resource();

        if let Some(extracted_world_width) = resources.get_ref::<WorldWidth>() {
            assert_eq!(extracted_world_width.0, 100.0);
        }
    }
    
    #[test]
    fn get_resource_mut() {
        let mut resources = initialize_resource();

        {
            let world_width: &mut WorldWidth = resources.get_mut::<WorldWidth>().unwrap();
            world_width.0 += 1.0;
        }

        let world_width = resources.get_ref::<WorldWidth>().unwrap();
        assert_eq!(world_width.0, 101.0);
    }

    #[test]
    fn remove_resource() {
        let mut resources = initialize_resource();
        resources.remove::<WorldWidth>();
        let world_width_type_id = TypeId::of::<WorldWidth>();
        
        assert!(!resources.data.contains_key(&world_width_type_id));
    }

    fn initialize_resource() -> Resources {
        let mut resources = Resources::default();
        let world_width = WorldWidth(100.0);

        resources.add(world_width);

        resources
    }

    struct WorldWidth(pub f32);
}