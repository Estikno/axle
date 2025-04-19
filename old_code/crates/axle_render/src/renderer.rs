use eyre::Result;
use sdl2::render::Canvas;
use sdl2::video::Window;

use axle_ecs::entities::query_entity::QueryEntity;
use axle_ecs::resources::Resources;
use axle_ecs::World;

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;
use crate::primitives::renderable::Renderable;

/// Initializes SDL and add to the ECS its resources (context, video subsystem, and canvas).
///
/// **Atention!**: This function should only be called once and it doesn't return the SDL2 elements,
/// it rather registers them in the ECS as resources.
pub fn new(world: &mut World) -> Result<()> {
    // Get config form the ECS's resources. If there is nothing return an error as the config must be
    // there.
    let config = world
        .get_resource::<RenderConfig>()
        .ok_or(CustomErrors::RenderConfigNotRegistered)?;

    // SDL initialization
    let sdl_context = sdl2::init().map_err(|_| CustomErrors::Sdl2InitError)?;
    let video_subsystem = sdl_context
        .video()
        .map_err(|_| CustomErrors::Sdl2CreateWindowError)?;
    let window = video_subsystem
        .window(&config.title, config.resolution.0, config.resolution.1)
        .position_centered()
        .build()
        .map_err(|_| CustomErrors::Sdl2CreateWindowError)?;
    let canvas = window
        .into_canvas()
        .build()
        .map_err(|_| CustomErrors::Sdl2CreateCanvasError)?;

    // Resgiter SDL2 component as resources so that they are available in the ECS
    world.add_resource(sdl_context);
    world.add_resource(video_subsystem);
    world.add_resource(canvas);

    // Register the main rendering function in the ECS
    world.create_system(&render);
    // .with_component::<Transform>()?
    // .with_component::<Box<dyn Renderable>>()?;

    Ok(())
}

// TODO:: Finish the render function
fn render(entities: &[QueryEntity], resources: &mut Resources) -> Result<()> {
    let canvas = resources.get_mut::<Canvas<Window>>().unwrap();

    canvas.clear();

    // for entities in entities {
    //     let transform = entities.get_component::<Transform>()?;
    //     let renderable = entities.get_component::<Box<dyn Renderable>>()?;
    //
    //     renderable.render(canvas, 600);
    // }

    canvas.present();

    Ok(())
}
