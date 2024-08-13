use axle2D::ecs::game_object::GameObject;
use axle2D::engine::Vector2;
use axle2D::engine::Transform;
use axle2D::physics::rigid_body::RigidBody;

use axle2D::physics::shape::Shape;
use sdl2::gfx::primitives::DrawRenderer;
//sdl2
use sdl2::pixels::Color;
use sdl2::rect::Rect;

pub struct Rectangle {
    transform: Transform,
    rigidbody: Option<RigidBody>
}

impl Rectangle {
    pub fn new() -> Rectangle {
        Rectangle {
            rigidbody: None,
            transform: Transform::default()
        }
    }

    pub fn set_rigidbody(&mut self) {
        self.rigidbody = Some(
            RigidBody::new(
                1.0, 
                1.0, 
                false, Shape::Circle { radius: 20.0 }
            )
        );
    }
}

impl GameObject for Rectangle {
    fn render(&self, canvas: &mut sdl2::render::Canvas<sdl2::video::Window>) {
        canvas.set_draw_color(Color::RED);
        //canvas.fill_rect(Rect::new(self.transform.position.x as i32, self.transform.position.y as i32, 100, 100)).ok().unwrap_or_default();
        if self.rigidbody.is_some() {
            if let Shape::Circle { radius } = &self.rigidbody.as_ref().unwrap().shape {
                canvas.filled_circle(self.transform.position.x as i16, self.transform.position.y as i16, radius.clone() as i16, Color::RED).unwrap_or_default();
            }
        }
    }

    fn get_position(&self) -> Vector2 {
        self.transform.position
    }

    fn transform(&self) -> &Transform {
        &self.transform
    }

    fn transform_mut(&mut self) -> &mut Transform {
        &mut self.transform
    }
}