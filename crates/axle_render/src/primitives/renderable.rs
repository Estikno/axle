use axle_math::vector::Vector2;

pub trait Renderable {
    /// Main render function to render
    fn render(&self);
    /// Get the position of the renderable
    fn get_position(&self) -> Vector2;
    /// Same as `get_position` but the coordinate system has its origin
    /// on the top left corner of the window (which is used for rendering)
    fn get_converted_position(&self) -> Vector2;
    /// Gets the vertices of the renderable in local space not accounting the
    /// rotation of the renderable.
    ///
    /// If the renderable does not have vertices, like an ellipse,
    /// circle, etc., returns `None`.
    fn get_vertices(&self) -> Option<Vec<Vector2>>;
    /// Same as `get_vertices` but it accounts for rotation
    fn get_transformed_vertices(&self) -> Option<Vec<Vector2>>;
}
