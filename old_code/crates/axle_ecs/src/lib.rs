//! Axle_ECS is a simple Entity Component System (ECS) mainly designed for the axle_engine game engine.

// mods
pub mod resources;
pub mod entities;
pub mod custom_errors;
pub mod world;
pub mod systems;

//re-exports
pub use world::World;
pub use custom_errors::CustomErrors;

pub mod prelude {
    pub use crate::world::World;
    pub use crate::custom_errors::CustomErrors;

    //types
    pub use crate::entities::{Component, Components};
    pub use crate::entities::query::{QueryComponents, QueryIndexes};
    pub use crate::entities::query_entity::QueryEntity;
    pub use crate::systems::{SystemComponents, SystemFunction};
}