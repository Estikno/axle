use std::time::Duration;

pub struct Resolution {
    pub width: u32,
    pub height: u32
}

pub struct GlobalConfig {
    pub resolution: Resolution,
    pub title: String,
    pub gravity: f32,
    pub fps: i32,
    pub frame_delay: Duration
}

impl GlobalConfig {
    pub fn new(resolution: Resolution, title: String, gravity: f32, fps: i32) -> Self {
        GlobalConfig {
            resolution,
            title,
            gravity,
            fps,
            frame_delay: Duration::from_secs_f32(1.0 / fps as f32)
        }
    }
}