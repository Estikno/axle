use crate::engine::input::Input;
use crate::engine::Transform;
use crate::engine::Vector2;
use crate::physics::shape::Shape;
use crate::physics::rigid_body::RigidBody;

/// Trait for objects in the game.
pub trait GameObject {
    /// Called when the game object is first created.
    fn start(&mut self) {
    }

    /// Called every frame to update the game object.
    ///
    /// # Arguments
    ///
    /// * `_delta_time` - The time elapsed since the last frame in seconds.
    /// * `_input` - The current input state.
    fn update(&mut self, _delta_time: f32, _input: &Input) {
    }

    /// Get the current position of the game object.
    fn get_position(&self) -> Vector2;

    /// Get a reference to the game object's transform.
    fn transform(&self) -> &Transform;

    /// Get a mutable reference to the game object's transform.
    fn transform_mut(&mut self) -> &mut Transform;

    /// Get a reference to the game object's shape.
    fn get_shape(&self) -> &Shape;

    /// Get a reference to the game object's rigid body, if it has one.
    fn get_rigidbody(&self) -> Option<&RigidBody>;
}