use thiserror::Error;

/// Custom errors that can occur when using the renderer
#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error(
        "Attempting to access a render config that has not been registered in the ECS's resources."
    )]
    RenderConfigNotRegistered,

    //sdl2 errors
    #[error("SDL2 could not initialize.")]
    Sdl2InitError,
    #[error("SDL2 could not create a window")]
    Sdl2CreateWindowError,
    #[error("SDL2 could not create a video subsystem")]
    Sdl2CreateVideoSubSystemError,
    #[error("SDL2 could not create a canvas")]
    Sdl2CreateCanvasError,
}
