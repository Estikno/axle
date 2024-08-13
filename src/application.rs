use std::rc::Rc;
use std::time::Instant;

use crate::config::GlobalConfig;
use crate::view::renderer::Renderer;
use crate::engine::input::Input;
use crate::ecs::game_object::GameObject;

pub struct App {
    pub config: Rc<GlobalConfig>,
    renderer: Renderer,
    pub input: Input,
    pub objects: Vec<Box<dyn GameObject>>
}

impl App {
    pub fn new(config: Rc<GlobalConfig>) -> Self {
        let renderer = Renderer::new(Rc::clone(&config));

        Self {
            input: Input::new(&renderer.sdl_context),
            config,
            renderer,
            objects: Vec::new()
        }
    }

    pub fn start(&self) {

    }

    pub fn update(&mut self) -> (bool, Instant) {
        let frame_start = Instant::now();

        //input
        self.input.update();
        let input_process = self.input.process_event();

        //render
        self.renderer.render(&mut self.objects);
        
        //return results
        (input_process, frame_start)
    }

    pub fn wait(&self, frame_start: Instant) {
        // Control frame time
        let frame_time = frame_start.elapsed();
        if frame_time < self.config.frame_delay {
            std::thread::sleep(self.config.frame_delay - frame_time);
        }
    }

    pub fn add_object(&mut self, object: Box<dyn GameObject>) {
        self.objects.push(object);
    }
}
