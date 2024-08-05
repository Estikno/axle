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