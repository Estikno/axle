use axle_ecs::World;
use axle_math::coordinate_system::convert_vector_y;
use axle_math::{transform::Transform, vector::Vector2};
use sdl2::gfx::primitives::DrawRenderer;

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;
use crate::primitives::renderable::Renderable;

// sdl2
use sdl2::pixels::Color;
use sdl2::render::Canvas;
use sdl2::video::Window;

pub struct Triangle {
    position: Vector2,
    vertices: [Vector2; 3],
    transformed_vertices: [Vector2; 3],
    color: Color,
}

impl Renderable for Triangle {
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
            self.transformed_vertices[i] =
                transform.transform_point(&transform.transform_vector(&self.vertices[i]));
        }
    }

    fn render(&self, canvas: &mut Canvas<Window>) {
        canvas
            .filled_trigon(
                self.transformed_vertices[0].x as i16,
                self.transformed_vertices[0].x as i16,
                self.transformed_vertices[0].x as i16,
                self.transformed_vertices[0].x as i16,
                self.transformed_vertices[0].x as i16,
                self.transformed_vertices[0].x as i16,
                self.color,
            )
            .unwrap_or_default();
    }
}

impl Triangle {
    pub fn new(position: Vector2, vertices: [Vector2; 3], color: Color) -> Self {
        Self {
            position,
            vertices,
            transformed_vertices: vertices,
            color,
        }
    }
}
