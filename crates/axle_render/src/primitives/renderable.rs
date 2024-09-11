use axle_ecs::World;
use axle_math::{transform::Transform, vector::Vector2};

pub trait Renderable {
    /// Main render function to render
    fn render(&self);

    /// Get the position of the renderable, specifically, its center
    fn get_position(&self) -> Vector2;

    /// Same as `get_position` but the coordinate system has its origin
    /// on the top left corner of the window (which is used for rendering)
    fn get_converted_position(&self, world: &World) -> Vector2;

    /// Gets the vertices of the renderable in local space not accounting the
    /// rotation of the renderable.
    ///
    /// If the renderable does not have vertices, like an ellipse,
    /// circle, etc., returns `None`.
    fn get_vertices(&self) -> Option<Vec<Vector2>>;

    /// Same as `get_vertices` but it accounts for rotation
    fn get_transformed_vertices(&self) -> Option<Vec<Vector2>>;

    /// Positions the vertices of the renderable accounting the rotation
    /// of the transform given.
    ///
    /// If the renderable does not have vertices, like an ellipse,
    /// this function does nothing.
    fn transform_vertices(&mut self, transform: &Transform);
}
