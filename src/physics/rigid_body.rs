use crate::engine::Vector2;
use crate::physics::shape::Shape;

pub struct RigidBody<'a> {
    position: &'a mut Vector2,
    linear_velocity: Vector2,
    rotation: &'a mut f32,
    angular_velocity: f32,

    pub mass: f32,
    pub density: f32,
    pub restitution: f32,

    pub is_static: bool,

    pub shape: Shape
}

impl<'a> RigidBody<'a> {
    pub fn new(position: &'a mut Vector2, rotation: &'a mut f32, mass: f32, density: f32, restitution: f32, is_static: bool, shape: Shape) -> Self {
        Self {
            position,
            linear_velocity: Vector2::new(0.0, 0.0),
            rotation,
            angular_velocity: 0_f32,
            mass,
            density,
            restitution,
            is_static,
            shape
        }
    }
}