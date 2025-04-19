use eyre::Result;
use std::time::{Duration, Instant};

// Own crates
use axle_ecs::World;
use axle_render::{config::RenderConfig, renderer};

#[derive(Default)]
pub struct App {
    pub world: World,
}

impl App {
    /// Creates a new app.
    /// Does the same as Default
    pub fn new() -> Self {
        Self {
            world: World::new(),
        }
    }

    /// A function for starting the app the basic features
    pub fn start(&mut self) -> Result<()> {
        let config = RenderConfig {
            resolution: (1280, 720),
            title: String::from("Axle"),
        };

        self.world.add_resource(config);
        renderer::new(&mut self.world)
    }

    /// A function for updating the systems and mantaining the game loop
    pub fn update(&mut self) -> Result<()> {
        let frame_start = Instant::now();

        //excecute systems which require update
        self.world.run_all_systems()?;

        self.wait(frame_start);

        Ok(())
    }

    pub fn wait(&self, frame_start: Instant) {
        // Control frame time
        let frame_time = frame_start.elapsed();
        if frame_time < Duration::from_secs_f32(1.0 / 60.0) {
            std::thread::sleep(Duration::from_secs_f32(1.0 / 60.0) - frame_time);
        }
    }
}
