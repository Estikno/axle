use crate::engine::{Transform, Vector2};

/// Represents a shape in 2D space.
#[derive(Debug)]
pub enum Shape {
    /// A rectangle with the specified width and height.
    Rectangle { 
        /// Width of the rectangle
        width: f32, 
        /// Height of the rectangle
        height: f32,
        /// An array of vector2 representing the coordinates of the vertices. Rotation not aplied, for that get transformed_vertices variable.
        /// The order is: top-left, top-right, bottom-right, bottom-left.
        /// The coordinates are local with the central point of the system located at the center of the rectangle.
        vertices: [Vector2; 4], 
        /// An array of vector2 representing the coordinates of the vertices in which rotation is applied
        /// The order is: top-left, top-right, bottom-right, bottom-left.
        /// The coordinates are global
        tranformed_vertices: [Vector2; 4],
        /// A bool that indicates if it is necessary to update the transformed vertices
        transform_update_required: bool,
        /// The order of coordinates of the triangles that have to be drawn in order to display the rectangle
        triangles: [u8; 6],
    },
    /// A circle with the specified radius.
    Circle { radius: f32 }
}

impl Shape {
    pub fn create_rectangle(width: f32, height: f32) -> Shape {
        let left = -width / 2.0;
        let right = left + width;
        let bottom = -height / 2.0;
        let top = bottom + height;

        let vertices = [
            Vector2::new(left, top),
            Vector2::new(right, top),
            Vector2::new(right, bottom),
            Vector2::new(left, bottom),
        ];

        let triangles = [0, 1, 2, 0, 2, 3];

        Shape::Rectangle { width, height, vertices, tranformed_vertices: vertices, transform_update_required: true, triangles }
    }

    pub fn get_transform_vertices(&mut self, transform: &Transform) -> Option<[Vector2; 4]> {
        match self {
            Shape::Rectangle { tranformed_vertices, transform_update_required, vertices, .. } => {
                if *transform_update_required {
                    for i in 0..4 {
                        let v = vertices[i];
                        tranformed_vertices[i] = transform.transform_point(&transform.transform_vector(&v));
                    }
                }

                *transform_update_required = false;
                Some(tranformed_vertices.clone())
            },
            _ => None,
        }
    }

    /// Returns the area of the shape.
    ///
    /// # Returns
    /// The area of the shape.
    pub fn get_area(&self) -> f32 {
        match self {
            Shape::Rectangle { width, height, .. } => width * height,
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
    /// 
    /// The dimensions of the shape (width and height), or `None` if it is not a rectangle.
    pub fn get_dimensions(&self) -> Option<Vector2> {
        match self {
            Shape::Rectangle { width, height, .. } => Some(Vector2::new(*width, *height)),
            _ => None,
        }
    }

    pub fn update_transformed_vertices(&mut self) {
        match self {
            Shape::Rectangle { transform_update_required, .. } => {
                *transform_update_required = true;
            },
            _ => {}
        }
    }
}