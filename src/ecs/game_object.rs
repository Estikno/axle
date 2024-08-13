use crate::engine::Transform;
use crate::engine::Vector2;
use crate::physics::shape::Shape;

pub trait GameObject {
    fn start(&mut self) {
        
    }
    fn update(&mut self, delta_time: f32) {
        
    }

    fn get_position(&self) -> Vector2;
    //fn render(&self, canvas: &mut Canvas<Window>);
    fn transform(&self) -> &Transform;
    fn transform_mut(&mut self) -> &mut Transform;
    fn get_shape(&self) -> &Shape;
}
