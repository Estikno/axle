//! Axle_ECS is a simple Entity Component System (ECS) mainly designed for the axle_engine game engine.

// mods
pub mod resources;
pub mod entities;
pub mod custom_errors;
pub mod world;
pub mod systems;

//re-exports
pub use world::World;
