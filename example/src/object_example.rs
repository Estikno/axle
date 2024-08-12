use axle2D::ecs::game_object::GameObject;
use axle2D::engine::Vector2;

//sdl2
use sdl2::pixels::Color;
use sdl2::rect::Rect;

pub struct Rectangle {
    pub position: Vector2,
    pub size: Vector2,
}

impl GameObject for Rectangle {
    fn render(&self, canvas: &mut sdl2::render::Canvas<sdl2::video::Window>) {
        canvas.set_draw_color(Color::RED);
        canvas.fill_rect(Rect::new(self.position.x as i32, self.position.y as i32, self.size.x as u32, self.size.y as u32)).ok().unwrap_or_default();
    }

    fn get_position(&self) -> (f32, f32) {
        (self.position.x, self.position.y)
    }
}