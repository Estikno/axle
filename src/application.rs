use axle_ecs::World;
use std::time::{Duration, Instant};

pub struct App {
    pub world: World,
}

impl App {
    pub fn new() -> Self {
        Self {
            world: World::new(),
        }
    }

    pub fn start(&self) {}

    pub fn update(&mut self) {
        let frame_start = Instant::now();

        self.wait(frame_start);
    }

    pub fn wait(&self, frame_start: Instant) {
        // Control frame time
        let frame_time = frame_start.elapsed();
        if frame_time < Duration::from_secs_f32(1.0 / 60.0) {
            std::thread::sleep(Duration::from_secs_f32(1.0 / 60.0) - frame_time);
        }
    }
}
