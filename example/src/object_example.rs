use axle2D::ecs::game_object::GameObject;
use axle2D::engine::Vector2;
use axle2D::engine::Transform;
use axle2D::physics::rigid_body::RigidBody;
use axle2D::physics::shape::Shape;

pub struct Rectangle {
    transform: Transform,
    rigidbody: Option<RigidBody>,
    shape: Shape
}

impl Rectangle {
    pub fn new() -> Rectangle {
        Rectangle {
            rigidbody: None,
            transform: Transform::default(),
            shape: Shape::Circle { radius: 20_f32 }
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
}