use std::rc::Rc;

use crate::config::GlobalConfig;
use crate::view::renderer::Renderer;

pub struct App {
    pub config: Rc<GlobalConfig>,
    renderer: Renderer,
}

impl App {
    pub fn new(config: Rc<GlobalConfig>) -> Self {
        let renderer = Renderer::new(Rc::clone(&config));
        Self {
            config,
            renderer
        }
    }

    pub fn start(&self) {

    }

    pub fn update(&mut self) {
        self.renderer.render();
    }
}
