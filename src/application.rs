use std::rc::Rc;
use std::time::Instant;

use crate::config::GlobalConfig;
use crate::engine::Vector2;
use crate::view::renderer::Renderer;
use crate::engine::input::Input;
use crate::ecs::game_object::GameObject;
use crate::physics::collisions;

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

        //collisions
        let objects_len = self.objects.len();
        let mut object_cols: Vec<((usize, usize), (Vector2, f32))> = Vec::new();

        for i in 0..(objects_len - 1) {
            for j in (i+1)..objects_len {
                if let Some((normal, depth)) = 
                    collisions::intersect_circles(self.objects[i].get_position(), 20.0, self.objects[j].get_position(), 20.0) {
                    object_cols.push(((i, j), (normal, depth)));
                }
            }
        }

        for info in object_cols.into_iter() {
            let transform_mut_a = self.objects[info.0.0].transform_mut();
            transform_mut_a.position = transform_mut_a.position + (info.1.0 * info.1.1 / 2.0);

            let transform_mut_b = self.objects[info.0.1].transform_mut();
            transform_mut_b.position = transform_mut_b.position - (info.1.0 * info.1.1 / 2.0);
        }

        //update
        for object in &mut self.objects {
            object.update(1.0 / self.config.fps as f32, &self.input);
        }

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
