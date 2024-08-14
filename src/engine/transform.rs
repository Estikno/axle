use crate::engine::Vector2;

/// Struct representing a 2D transformation
///
/// This struct contains the position, rotation, and scale of a 2D object.
#[derive(Debug, Copy, Clone)]
pub struct Transform {
    /// Position of the object
    pub position: Vector2,
    /// Rotation of the object in degrees
    pub rotation: f32, // in degrees for now
    /// Scale of the object
    pub scale: Vector2,
}

impl Transform {
    /// Returns a default Transform with position (0, 0), rotation 0, and scale (1, 1)
    pub fn default() -> Self {
        Self {
            position: Vector2::zero(), // position at (0, 0)
            rotation: 0.0,            // rotation is 0 degrees
            scale: Vector2::one(),    // scale is (1, 1)¡
        }
    }

    /// Returns a new Transform with the given position, rotation, and scale
    ///
    /// # Arguments
    ///
    /// * `position` - The position of the object
    /// * `rotation` - The rotation of the object in degrees
    /// * `scale` - The scale of the object
    ///
    /// # Returns
    ///
    /// A new Transform with the given parameters.
    pub fn new(position: Vector2, rotation: f32, scale: Vector2) -> Self {
        Self {
            position,  // use the given position
            rotation, // use the given rotation
            scale,    // use the given scale
        }
    }
}

