use std::rc::Rc;

use crate::config::GlobalConfig;
use crate::view::renderer::Renderer;
use crate::engine::input::Input;

pub struct App {
    pub config: Rc<GlobalConfig>,
    renderer: Renderer,
    pub input: Input
}

impl App {
    pub fn new(config: Rc<GlobalConfig>) -> Self {
        let renderer = Renderer::new(Rc::clone(&config));

        Self {
            input: Input::new(&renderer.sdl_context),
            config,
            renderer,
        }
    }

    pub fn start(&self) {

    }

    pub fn update(&mut self) -> bool {
        self.renderer.render();
        
        self.input.update();
        self.input.process_event()
    }
}
