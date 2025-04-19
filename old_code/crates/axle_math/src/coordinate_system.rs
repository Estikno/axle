use crate::vector::Vector2;

/// Converts from a 2D coordinate system where the origin is located on the bottom left
/// corner to the top left corner of the window (mainly used for rendering)
///
/// # Arguments
/// * `y` - y coordinate on the previous coordinate system
/// * `window_height` - window height
///
/// # Returns
/// Converted y coordinate
pub fn convert_point_y(y: f32, window_height: u32) -> f32 {
    -y + (window_height as f32)
}

/// Same as `convert_point_y` but for `Vector2`
///
/// # Arguments
/// * `v` - Vector2
/// * `window_height` - window height
///
/// # Returns
/// Converted Vector2
pub fn convert_vector_y(v: &Vector2, window_height: u32) -> Vector2 {
    Vector2::new(v.x, convert_point_y(v.y, window_height))
}
