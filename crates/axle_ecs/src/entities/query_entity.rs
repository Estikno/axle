use std::{any::{Any, TypeId}, cell::{Ref, RefCell, RefMut}, rc::Rc};
use eyre::Result;

use crate::prelude::*;
use super::Entities;

type ExtractedComponents = Vec<Option<Rc<RefCell<dyn Any>>>>;

/// A struct that represents an entity in a query.
///
/// This struct is used in the query system to represent an entity that matches
/// a query.
///
/// It contains the id of the entity and a reference to the `Entities` struct
/// that it is a part of.
#[derive(Clone, Copy)]
pub struct QueryEntity<'a> {
    /// The id of the entity.
    pub id: usize,
    /// A reference to the `Entities` struct that this entity is a part of.
    entities: &'a Entities,
}

impl<'a> QueryEntity<'a> {
    /// Creates a new `QueryEntity` struct.
    ///
    /// # Arguments
    ///
    /// * `id` - The id of the entity.
    /// * `entities` - A reference to the `Entities` struct that this entity is a part of.
    ///
    /// # Returns
    ///
    /// A new `QueryEntity` struct.
    pub fn new(id: usize, entities: &'a Entities) -> Self {
        Self { id, entities }
    }

    /// Gets the components of type `T` from the `Entities` struct that this `QueryEntity` is a part of.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the components to get.
    ///
    /// # Returns
    ///
    /// A reference to the components of type `T`.
    ///
    /// # Errors
    ///
    /// If the component type `T` is not registered, an error is returned.
    fn extract_components<T: Any>(&self) -> Result<&ExtractedComponents> {
        let type_id = TypeId::of::<T>();
        self
            .entities
            .components
            .get(&type_id)
            .ok_or_else(|| CustomErrors::ComponentNotRegistered.into())
    }

    /// Gets a reference to the components of type `T` from the `Entities` struct that this
    /// `QueryEntity` is a part of.
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the components to get.
    ///
    /// # Returns
    ///
    /// A reference to the entity's component of type `T`.
    ///
    /// # Errors
    ///
    /// If the component type `T` is not in the entity, an error is returned.
    pub fn get_component<T: Any>(&self) -> Result<Ref<T>> {
        let components = self.extract_components::<T>()?;
        let borrowed_component = components[self.id]
            .as_ref()
            .ok_or(CustomErrors::ComponentDataDoesNotExist)?
            .borrow();

        Ok(
            Ref::map(borrowed_component, |any: &dyn Any| any.downcast_ref::<T>().unwrap())
        )
    }

    /// Gets a mutable reference to the component of type `T` from the entity pointed by this `QueryEntity`
    ///
    /// # Arguments
    ///
    /// * `T` - The type of the components to get.
    ///
    /// # Returns
    ///
    /// A mutable reference to the entity's component of type `T`.
    ///
    /// # Errors
    ///
    /// If the component type `T` is not in the entity, an error is returned.
    pub fn get_component_mut<T: Any>(&self) -> Result<RefMut<T>> {
        let components = self.extract_components::<T>()?;
        let borrowed_component = components[self.id]
            .as_ref()
            .ok_or(CustomErrors::ComponentDataDoesNotExist)?
            .borrow_mut();

        Ok(
            RefMut::map(borrowed_component, |any: &mut dyn Any| any.downcast_mut::<T>().unwrap())
        )
    }
}