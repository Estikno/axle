//! Axle_ECS is a simple Entity Component System (ECS) mainly designed for the axle_engine game engine.

// mods
pub mod resource;
pub mod entities;
pub mod custom_errors;
pub mod world;

//re-exports
pub use world::World;
