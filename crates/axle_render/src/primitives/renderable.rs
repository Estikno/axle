use axle_math::{transform::Transform, vector::Vector2};

// sdl2
use sdl2::render::Canvas;
use sdl2::video::Window;

pub trait Renderable {
    /// Renders the renderable
    ///
    /// # Arguments
    ///
    /// * `canvas` - The canvas to render on
    /// * `window_height` - The height of the window
    fn render(&self, canvas: &mut Canvas<Window>, window_height: u32);

    /// Get the position of the renderable
    fn get_position(&self) -> Vector2;

    /// Same as `get_position` but the coordinate system has its origin
    /// on the top left corner of the window and y upwards is negative
    /// (which is used for rendering)
    ///
    /// # Arguments
    ///
    /// * `world` - The world of the ECS
    /// * `window_height` - The height of the window
    ///
    /// # Returns
    ///
    /// The position of the renderable in the coordinate system
    fn get_converted_position(&self, window_height: u32) -> Vector2;

    /// Gets the vertices of the renderable in local space not accounting the
    /// rotation of the renderable.
    ///
    /// If the renderable does not have vertices, like an circle,
    /// circle, etc., returns `None`.
    fn get_vertices(&self) -> Option<Vec<Vector2>>;

    /// Same as `get_vertices` but it accounts for rotation and global position
    fn get_transformed_vertices(&self) -> Option<Vec<Vector2>>;

    /// Positions the vertices of the renderable accounting for rotation
    /// and position of the transform given.
    ///
    /// If the renderable does not have vertices, like a circle,
    /// this function does nothing.
    ///
    /// # Arguments
    ///
    /// * `transform` - This renderable's transform
    fn transform_vertices(&mut self, transform: &Transform);

    fn set_transform_vertices(&mut self, vertices: Vec<Vector2>);
}
