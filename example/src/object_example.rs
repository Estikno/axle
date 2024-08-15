use std::f32::consts::PI;

use axle2D::ecs::game_object::GameObject;
use axle2D::engine::Keycode;
use axle2D::engine::Vector2;
use axle2D::engine::Transform;
use axle2D::physics::rigid_body::RigidBody;
use axle2D::physics::shape::Shape;

pub struct Rectangle {
    transform: Transform,
    rigidbody: Option<RigidBody>,
    shape: Shape,
    movable: bool
}

impl Rectangle {
    pub fn new(initial_pos: Vector2, initial_rot: f32, initial_scale: Vector2, movable: bool) -> Rectangle {
        Rectangle {
            transform: Transform::new(initial_pos, initial_rot, initial_scale),
            rigidbody: None,
            shape: Shape::create_rectangle(10.0 * 3.0, 10.0 * 3.0),
            movable
        }
    }

    pub fn set_rigidbody(&mut self) {
        self.rigidbody = Some(
            RigidBody::new(
                1.0, 
                1.0, 
                false, 
                self.shape.clone()
            )
        );
    }
}

impl GameObject for Rectangle {
    fn get_position(&self) -> Vector2 {
        self.transform.position
    }

    fn transform(&self) -> &Transform {
        &self.transform
    }

    fn transform_mut(&mut self) -> &mut Transform {
        &mut self.transform
    }

    fn get_shape(&self) -> &Shape {
        &self.shape
    }

    fn update(&mut self, delta_time: f32, input: &axle2D::engine::input::Input) {
        self.transform.rotation += PI / 50.0;
        self.shape.get_transform_vertices(&self.transform);

        if !self.movable {
            return;
        }
        
        if input.is_key_held(Keycode::W) {
            self.transform.position.y -= 2.0;
        }
        if input.is_key_held(Keycode::S) {
            self.transform.position.y += 2.0;
        }
        if input.is_key_held(Keycode::A) {
            self.transform.position.x -= 2.0;
        }
        if input.is_key_held(Keycode::D) {
            self.transform.position.x += 2.0;
        }
    }

    fn get_rigidbody(&self) -> Option<&RigidBody> {
        self.rigidbody.as_ref()
    }
}