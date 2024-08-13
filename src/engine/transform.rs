use crate::engine::Vector2;

#[derive(Debug, Copy, Clone)]
pub struct Transform {
    pub position: Vector2,
    pub rotation: f32, // in degrees for now
    pub scale: Vector2,
}

impl Transform {
    pub fn default() -> Self {
        Self {
            position: Vector2::zero(),
            rotation: 0.0,
            scale: Vector2::one(),
        }
    }

    pub fn new(position: Vector2, rotation: f32, scale: Vector2) -> Self {
        Self {
            position,
            rotation,
            scale,
        }
    }
}