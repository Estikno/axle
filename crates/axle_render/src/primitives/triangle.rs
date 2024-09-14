use axle_math::coordinate_system::{convert_point_y, convert_vector_y};
use axle_math::{transform::Transform, vector::Vector2};

use crate::primitives::renderable::Renderable;

// sdl2
use sdl2::gfx::primitives::DrawRenderer;
use sdl2::pixels::Color;
use sdl2::render::Canvas;
use sdl2::video::Window;

/// Struct that represents a triangle.
/// Main renderable used for rendering practically everything
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

    fn get_converted_position(&self, window_height: u32) -> Vector2 {
        convert_vector_y(&self.position, window_height)
    }

    fn get_vertices(&self) -> Option<Vec<Vector2>> {
        Some(self.vertices.to_vec())
    }

    fn get_transformed_vertices(&self) -> Option<Vec<Vector2>> {
        Some(self.transformed_vertices.to_vec())
    }

    fn transform_vertices(&mut self, transform: &Transform) {
        for i in 0..3 {
            self.transformed_vertices[i] =
                transform.transform_point(&transform.transform_vector(&self.vertices[i]));
        }
    }

    fn render(&self, canvas: &mut Canvas<Window>, window_height: u32) {
        canvas
            .filled_trigon(
                self.transformed_vertices[0].x as i16,
                convert_point_y(self.transformed_vertices[0].y, window_height) as i16,
                self.transformed_vertices[1].x as i16,
                convert_point_y(self.transformed_vertices[1].y, window_height) as i16,
                self.transformed_vertices[2].x as i16,
                convert_point_y(self.transformed_vertices[2].y, window_height) as i16,
                self.color,
            )
            .unwrap_or_default();
    }

    fn set_transform_vertices(&mut self, vertices: Vec<Vector2>) {
        self.transformed_vertices = vertices.try_into().unwrap();
    }
}

impl Triangle {
    /// Creates a new triangle with the given position and vertices
    ///
    /// # Arguments
    ///
    /// * `position` - The position of the triangle
    /// * `vertices` - The vertices of the triangle
    /// * `color` - The color of the triangle
    ///
    /// # Returns
    ///
    /// A new triangle
    pub fn new(position: Vector2, vertices: [Vector2; 3], color: Color) -> Self {
        Self {
            position,
            vertices,
            transformed_vertices: vertices,
            color,
        }
    }
}
