use axle_ecs::World;
use axle_math::coordinate_system::convert_vector_y;
use axle_math::{transform::Transform, vector::Vector2};
use std::f32::consts::PI;

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;
use crate::primitives::renderable::Renderable;

// sdl2
use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Ellipse {
    pub position: Vector2,
    pub radius_x: f32,
    pub radius_y: f32,

    vertices: [Vector2; 8],
    transformed_vertices: [Vector2; 8],
}

impl Renderable for Ellipse {
    fn get_position(&self) -> Vector2 {
        self.position
    }

    fn get_converted_position(&self, world: &World) -> Vector2 {
        convert_vector_y(
            &self.position,
            world
                .get_resource::<RenderConfig>()
                .ok_or(CustomErrors::RenderConfigNotRegistered)
                .unwrap()
                .resolution
                .1,
        )
    }

    fn get_vertices(&self) -> Option<Vec<Vector2>> {
        Some(self.vertices.to_vec())
    }

    fn get_transformed_vertices(&self) -> Option<Vec<Vector2>> {
        Some(self.transformed_vertices.to_vec())
    }

    fn transform_vertices(&mut self, transform: &Transform) {
        for i in 0..self.vertices.len() {
            self.transformed_vertices[i] = transform.transform_vector(&self.vertices[i]);
        }
    }

    fn render(&self, canvas: &mut Canvas<Window>) {
        todo!()
    }
}

impl Ellipse {
    pub fn new(position: Vector2, radius_x: f32, radius_y: f32) -> Self {
        let mut vertices_pos: [Vector2; 8] = [Vector2::ZERO; 8];

        (0..8).for_each(|i| {
            let theta = 2.0 * PI * (i as f32) / 8.0;
            vertices_pos[i] = Vector2::new(radius_x * theta.cos(), radius_y * theta.sin());
        });

        Self {
            position,
            radius_x,
            radius_y,
            vertices: vertices_pos,
            transformed_vertices: vertices_pos,
        }
    }
}
