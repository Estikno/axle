use std::rc::Rc;

use crate::config::GlobalConfig;
use crate::ecs::game_object::GameObject;
use crate::physics::shape::Shape;

//sdl2
use sdl2::Sdl;
use sdl2::render::Canvas;
use sdl2::video::Window;
use sdl2::pixels::Color;
use sdl2::VideoSubsystem;
use sdl2::gfx::primitives::DrawRenderer;
use sdl2::rect::Rect;

pub struct Renderer {
    //main parameters
    pub sdl_context: Sdl,
    pub video_subsystem: VideoSubsystem,
    //pub window: Window,
    pub canvas: Canvas<Window>,

    //other not as important parameters
    pub background_color: Color,
    pub config: Rc<GlobalConfig>
}

impl Renderer {
    pub fn new(config: Rc<GlobalConfig>) -> Self {
        //sdl2 initialization
        let sdl_context = sdl2::init().unwrap();
        let video_subsystem = sdl_context.video().unwrap();
        let window = video_subsystem.window(&config.title, config.resolution.width, config.resolution.height)
            .position_centered()
            .build()
            .unwrap();
        let canvas = window.into_canvas().build().unwrap();

        Self {
            sdl_context,
            video_subsystem,
            //window,
            canvas,
            background_color: Color::GRAY,
            config
        }
    }

    pub fn render(&mut self, objects: &mut Vec<Box<dyn GameObject>>) {
        //background
        self.canvas.set_draw_color(self.background_color);
        self.canvas.clear();

        //obsticles
        //player
        for object in objects {
            //update object
            object.update(1_f32 / self.config.fps as f32);

            //render it
            //object.render(&mut self.canvas);
            let shape = object.get_shape();
            
            match shape {
                Shape::Circle { radius } => {
                    self.canvas.filled_circle(object.get_position().x as i16, object.get_position().y as i16, radius.clone() as i16, Color::RED).unwrap_or_default();
                },
                Shape::Rectangle { width, height } => {
                    self.canvas.fill_rect(Rect::new(object.get_position().x as i32, object.get_position().y as i32, width.clone() as u32, height.clone() as u32)).unwrap_or_default();
                }
            }
        }

        //present the render
        self.canvas.present();
    }
}