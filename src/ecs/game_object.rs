//sld2
use sdl2::{render::Canvas, video::Window};

pub trait GameObject {
    fn start(&mut self) {
        
    }
    fn update(&mut self, delta_time: f32) {
        
    }
    fn get_position(&self) -> (f32, f32);
    fn render(&self, canvas: &mut Canvas<Window>);
}
