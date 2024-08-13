use crate::engine::Vector2;
use crate::physics::shape::Shape;

pub struct RigidBody {
    linear_velocity: Vector2,
    angular_velocity: f32,

    pub mass: f32,
    pub restitution: f32,

    pub is_static: bool,

    pub shape: Shape
}

impl RigidBody {
    pub fn new(mass: f32, restitution: f32, is_static: bool, shape: Shape) -> Self {
        Self {
            linear_velocity: Vector2::new(0.0, 0.0),
            angular_velocity: 0_f32,
            mass,
            restitution: restitution.clamp(0.0, 1.0), //clamp between 0 and 1
            is_static,
            shape
        }
    }
}