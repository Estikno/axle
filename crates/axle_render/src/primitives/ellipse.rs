use axle_ecs::World;
use axle_math::coordinate_system::convert_vector_y;
use axle_math::{transform::Transform, vector::Vector2};

use crate::config::RenderConfig;
use crate::custom_errors::CustomErrors;
use crate::primitives::renderable::Renderable;

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

    fn render(&self) {
        todo!()
    }
}
