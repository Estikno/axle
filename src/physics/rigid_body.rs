use axle_math::vector::Vector2;
use crate::physics::shape::Shape;

/// A rigid body is an object that does not deform when it moves.
/// It is the simplest type of physics object in the engine.
pub struct RigidBody {
    /// The linear velocity of the rigid body.
    pub linear_velocity: Vector2,
    /// The angular velocity of the rigid body.
    pub angular_velocity: f32,

    /// The mass of the rigid body.
    pub mass: f32,
    /// The restitution of the rigid body. This is a value between 0 and 1 that
    /// represents how bouncy the rigid body is. A value of 0 means that the
    /// rigid body is not bouncy at all, while a value of 1 means that the rigid
    /// body is perfectly bouncy.
    pub restitution: f32,

    /// Whether the rigid body is static or not. A static rigid body is one
    /// that does not move when other objects collide with it.
    pub is_static: bool,
}

impl RigidBody {
    /// Creates a new RigidBody.
    ///
    /// # Arguments
    ///
    /// * `mass` - The mass of the RigidBody.
    /// * `restitution` - The restitution of the RigidBody. This value is clamped between 0 and 1.
    /// * `is_static` - Whether the RigidBody is static or not.
    /// * `shape` - The shape of the RigidBody.
    ///
    /// # Returns
    ///
    /// A new RigidBody.
    pub fn new(mass: f32, restitution: f32, is_static: bool) -> Self {
        Self {
            linear_velocity: Vector2::new(0.0, 0.0), // initialize linear velocity to (0, 0)
            angular_velocity: 0_f32, // initialize angular velocity to 0
            mass,
            restitution: restitution.clamp(0.0, 1.0), // clamp restitution between 0 and 1
            is_static
        }
    }
}