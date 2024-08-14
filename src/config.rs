use std::time::Duration;

/// Structure representing the resolution of the game window.
#[derive(Debug)]
pub struct Resolution {
    /// Width of the game window.
    pub width: u32,
    /// Height of the game window.
    pub height: u32
}

/// Structure containing the global configuration of the game.
#[derive(Debug)]
pub struct GlobalConfig {
    /// Resolution of the game window.
    pub resolution: Resolution,
    /// Title of the game window.
    pub title: String,
    /// Gravity of the game world.
    pub gravity: f32,
    /// Target frames per second of the game.
    pub fps: i32,
    /// Delay between frames based on the target FPS.
    pub frame_delay: Duration
}

impl GlobalConfig {
    /// Creates a new instance of `GlobalConfig`.
    ///
    /// # Arguments
    ///
    /// * `resolution` - The resolution of the game window.
    /// * `title` - The title of the game window.
    /// * `gravity` - The gravity of the game world.
    /// * `fps` - The target frames per second of the game.
    ///
    /// # Returns
    ///
    /// A new instance of `GlobalConfig`.
    pub fn new(resolution: Resolution, title: String, gravity: f32, fps: i32) -> Self {
        Self {
            resolution,
            title,
            gravity,
            fps,
            frame_delay: Duration::from_secs_f32(1.0 / fps as f32)
        }
    }
}
