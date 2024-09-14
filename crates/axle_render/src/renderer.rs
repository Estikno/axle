use axle_ecs::entities::query_entity::QueryEntity;
use axle_ecs::World;
use axle_math::transform::Transform;
use eyre::Result;

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;
use crate::primitives::renderable::Renderable;

//sdl2
use sdl2::render::Canvas;
use sdl2::video::Window;
use sdl2::Sdl;
use sdl2::VideoSubsystem;

pub struct Renderer {
    sdl_context: Sdl,
    canvas: Canvas<Window>,
    video_subsystem: VideoSubsystem,
}

impl Renderer {
    pub fn new(world: &mut World) -> Result<Self> {
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

        // Register render function in the ECS
        world
            .create_system(&Renderer::render)
            .with_component::<Transform>()?
            .with_component::<Box<dyn Renderable>>()?;

        Ok(Self {
            sdl_context,
            video_subsystem,
            canvas,
        })
    }

    pub fn render(entities: &Vec<QueryEntity>) -> Result<()> {
        for entities in entities {
            let transform = entities.get_component::<Transform>()?;
        }

        Ok(())
    }
}
