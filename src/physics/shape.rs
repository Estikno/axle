use crate::engine::Vector2;

/// Represents a shape in 2D space.
#[derive(Clone)]
pub enum Shape {
    /// A rectangle with the specified width and height.
    Rectangle { width: f32, height: f32 },
    /// A circle with the specified radius.
    Circle { radius: f32 }
}

impl Shape {
    /// Returns the area of the shape.
    ///
    /// # Returns
    /// The area of the shape.
    pub fn get_area(&self) -> f32 {
        match self {
            Shape::Rectangle { width, height } => width * height,
            Shape::Circle { radius } => std::f32::consts::PI * radius * radius
        }
    }

    /// Returns the radius of the shape, if it is a circle.
    ///
    /// # Returns
    /// The radius of the shape, or `None` if it is not a circle.
    pub fn get_radius(&self) -> Option<f32> {
        match self {
            Shape::Circle { radius } => Some(*radius),
            _ => None,
        }
    }

    /// Returns the dimensions of the shape, if it is a rectangle.
    ///
    /// # Returns
    /// The dimensions of the shape (width and height), or `None` if it is not a rectangle.
    pub fn get_dimensions(&self) -> Option<Vector2> {
        match self {
            Shape::Rectangle { width, height } => Some(Vector2::new(*width, *height)),
            _ => None,
        }
    }
}