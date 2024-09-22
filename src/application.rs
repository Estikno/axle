use std::rc::Rc;
use std::time::Instant;

use sdl2::pixels::Color;

use crate::config::GlobalConfig;
use crate::engine::input::Input;
use crate::physics::collisions;
use crate::view::renderer::Renderer;

// pub struct App {
//     renderer: Renderer,
//     pub input: Input,
// }
//
// impl App {
//     pub fn new() -> Self {
//         let renderer = Renderer::new();
//
//         Self {
//             input: Input::new(&renderer.sdl_context),
//             renderer,
//         }
//     }
//
//     pub fn start(&self) {}
//
//     pub fn update(&mut self) -> (bool, Instant) {
//         let frame_start = Instant::now();
//
//         //input
//         self.input.update();
//         let input_process = self.input.process_event();
//
//         //collisions
//         /*let objects_len = self.objects.len();
//         let mut object_cols: Vec<((usize, usize), (Vector2, f32))> = Vec::new();
//
//         //reset color
//         for object in &mut self.objects {
//             object.get_shape_mut().set_color(Color::GREEN);
//         }
//
//         for i in 0..(objects_len - 1) {
//             if let None = self.objects[i].get_rigidbody() {
//                 continue;
//             }
//
//             for j in (i+1)..objects_len {
//                 if let None = self.objects[j].get_rigidbody() {
//                     continue;
//                 }
//
//                 if collisions::intersect_polygons(&self.objects[i].get_shape().get_transform_vertices().unwrap(), &self.objects[j].get_shape().get_transform_vertices().unwrap()) {
//                     self.objects[i].get_shape_mut().set_color(Color::RED);
//                     self.objects[j].get_shape_mut().set_color(Color::RED);
//                 }
//
//                 //circle collision
//                 /*if let Some((normal, depth)) =
//                     collisions::intersect_circles(self.objects[i].get_position(), 20.0, self.objects[j].get_position(), 20.0) {
//                     object_cols.push(((i, j), (normal, depth)));
//                 }*/
//             }
//         }*/
//
//         //go through the collisions vector and apply the collision to each pair of objects
//         /*for info in object_cols.into_iter() {
//             let transform_mut_a = self.objects[info.0.0].transform_mut();
//             transform_mut_a.position = transform_mut_a.position + (info.1.0 * info.1.1 / 2.0);
//
//             let transform_mut_b = self.objects[info.0.1].transform_mut();
//             transform_mut_b.position = transform_mut_b.position - (info.1.0 * info.1.1 / 2.0);
//         }*/
//
//         //update
//         /*for object in &mut self.objects {
//             object.update(1.0 / self.config.fps as f32, &self.input);
//         }*/
//
//         //render
//         //self.renderer.render(&mut self.objects);
//
//         //return results
//         (input_process, frame_start)
//     }
//
//     pub fn wait(&self, frame_start: Instant) {
//         // Control frame time
//         let frame_time = frame_start.elapsed();
//         if frame_time < self.config.frame_delay {
//             std::thread::sleep(self.config.frame_delay - frame_time);
//         }
//     }
// }
