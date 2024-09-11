use axle_ecs::World;
use std::rc::Rc;

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;

//sdl2
use sdl2::render::Canvas;
use sdl2::video::Window;
use sdl2::Sdl;
use sdl2::VideoSubsystem;

pub struct Renderer {
    sdl_context: Sdl,
    canvas: Canvas<Window>,
    video_subsystem: VideoSubsystem,
    world: Rc<World>,
}

impl Renderer {
    pub fn new(world: Rc<World>) -> Self {
        // Get config form the ecs's resources. If there is nothing panic as the config must be
        // there.
        let config = world
            .get_resource::<RenderConfig>()
            .ok_or(CustomErrors::RenderConfigNotRegistered)
            .unwrap();

        // SDL initialization
        let sdl_context = sdl2::init().unwrap();
        let video_subsystem = sdl_context.video().unwrap();
        let window = video_subsystem
            .window(&config.title, config.resolution.0, config.resolution.1)
            .position_centered()
            .build()
            .unwrap();
        let canvas = window.into_canvas().build().unwrap();

        Self {
            sdl_context,
            video_subsystem,
            canvas,
            world,
        }
    }
}
