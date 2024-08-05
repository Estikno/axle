use std::ops::{Add, Div, Mul, Sub};


/// A 2-dimensional vector.
///
/// This struct represents a 2-dimensional vector with `x` and `y` coordinates.
#[derive(Debug, Clone, Copy)]
pub struct Vector2 {
    /// The x-coordinate of the vector.
    pub x: f32,
    /// The y-coordinate of the vector.
    pub y: f32
}

impl Add for Vector2 {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y
        }
    }
}

impl Sub for Vector2 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y
        }
    }
}

impl Mul<f32> for Vector2 {
    type Output = Self;

    fn mul(self, scalar: f32) -> Self::Output {
        Self {
            x: self.x * scalar,
            y: self.y * scalar
        }
    }
}

impl Div<f32> for Vector2 {
    type Output = Self;

    fn div(self, scalar: f32) -> Self::Output {
        Self {
            x: self.x / scalar,
            y: self.y / scalar
        }
    }
}

//basic implemetations
impl Vector2 {
    /// Create a new `Vector2` with the given x and y coordinates.
    ///
    /// # Arguments
    ///
    /// * `x` - The x-coordinate of the `Vector2`.
    /// * `y` - The y-coordinate of the `Vector2`.
    ///
    /// # Returns
    ///
    /// A new `Vector2` with the given coordinates.
    pub fn new(x: f32, y: f32) -> Self {
        Self { x, y }
    }

    /// Create a `Vector2` pointing downwards.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(0.0, -1.0)`.
    pub fn down() -> Self {
        Self {
            x: 0.0,
            y: -1.0
        }
    }

    /// Create a `Vector2` pointing leftwards.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(-1.0, 0.0)`.
    pub fn left() -> Self {
        Self {
            x: -1.0,
            y: 0.0
        }
    }

    /// Create a `Vector2` pointing rightwards.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(1.0, 0.0)`.
    pub fn right() -> Self {
        Self {
            x: 1.0,
            y: 0.0
        }
    }

    /// Create a `Vector2` pointing upwards.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(0.0, 1.0)`.
    pub fn up() -> Self {
        Self {
            x: 0.0,
            y: 1.0
        }
    }

    /// Create a `Vector2` with all coordinates set to zero.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(0.0, 0.0)`.
    pub fn zero() -> Self {
        Self {
            x: 0.0,
            y: 0.0
        }
    }

    /// Create a `Vector2` with all coordinates set to one.
    ///
    /// # Returns
    ///
    /// A `Vector2` with coordinates `(1.0, 1.0)`.
    pub fn one() -> Self {
        Self {
            x: 1.0,
            y: 1.0
        }
    }
}

//other more complex implementations
impl Vector2 {
    /// Returns the squared magnitude of the vector.
    ///
    /// # Returns
    ///
    /// The squared magnitude of the vector.
    pub fn sqr_magnitude(&self) -> f32 {
        // Calculate the sum of the squares of the x and y coordinates.
        self.x * self.x + self.y * self.y
    }

    /// Returns the magnitude of the vector.
    /// If you only need to compare magnitudes of some vectors, you can compare squared magnitudes of them using sqrMagnitude (computing squared magnitudes is faster).
    ///
    /// # Returns
    ///
    /// The magnitude of the vector.
    pub fn magnitude(&self) -> f32 {
        // Calculate the square root of the squared magnitude.
        self.sqr_magnitude().sqrt()
    }

    /// Returns a new vector with the same direction as the current vector, but with a magnitude of 1.
    ///
    /// # Returns
    ///
    /// A new vector with a magnitude of 1.
    /// 
    /// # Panics
    ///
    /// This function panics if the current vector has a magnitude of 0.
    pub fn normalized(&self) -> Self {
        // Calculate the magnitude of the vector.
        let magnitude = self.magnitude();

        // Create a new vector with the same direction as the current vector, but with a magnitude of 1.
        Self {
            x: self.x / magnitude,
            y: self.y / magnitude
        }
    }

    /// Modifies the current vector to have a magnitude of 1.
    ///
    /// # Panics
    ///
    /// This function panics if the current vector has a magnitude of 0.
    pub fn normalize(&mut self) {
        // Calculate the magnitude of the vector.
        let magnitude = self.magnitude();

        // Divide the x and y coordinates of the current vector by the magnitude.
        self.x /= magnitude;
        self.y /= magnitude;
    }

    /// Modifies the current vector to have a maximum magnitude.
    ///
    /// # Arguments
    ///
    /// * `max_magnitude` - The maximum magnitude of the vector.
    ///
    /// # Panics
    ///
    /// This function panics if the current vector has a magnitude of 0.
    pub fn clamp_magnitude(&mut self, max_magnitude: f32) {
        // Calculate the factor by which the current vector needs to be multiplied to have a maximum magnitude.
        let f = max_magnitude / self.magnitude();

        // Multiply the x and y coordinates of the current vector by the factor.
        self.x *= f;
        self.y *= f;
    }

    /// Returns a new vector that is perpendicular to the current vector.
    ///
    /// # Returns
    ///
    /// A new vector that is perpendicular to the current vector. The 
    /// result is always rotated 90-degrees in a counter-clockwise direction for a 2D coordinate system where the positive Y axis goes up.
    pub fn perpendicular(&self) -> Self {
        Self {
            x: -self.y,
            y: self.x
        }
    }
}

//static implementations
impl Vector2 {
    /// Calculates the angle in degrees between two vectors.
    ///
    /// # Arguments
    ///
    /// * `from` - The starting vector.
    /// * `to` - The ending vector.
    ///
    /// # Returns
    ///
    /// The angle in degrees between the two vectors.
    pub fn angle(from: &Vector2, to: &Vector2) -> f32 {
        // Calculate the dot product of the two vectors divided by the product of their magnitudes.
        // Then take the arccosine of the result and convert it to degrees.
        (Vector2::dot(from, to)/(from.magnitude() * to.magnitude())).acos().to_degrees()
    }

    /// Calculates the dot product of two vectors.
    ///
    /// # Arguments
    ///
    /// * `lhs` - The first vector.
    /// * `rhs` - The second vector.
    ///
    /// # Returns
    ///
    /// The dot product of the two vectors.
    pub fn dot(lhs: &Vector2, rhs: &Vector2) -> f32 {
        lhs.x * rhs.x + lhs.y * rhs.y
    }

    /// Calculates the euclidean distance between two vectors.
    ///
    /// # Arguments
    ///
    /// * `a` - The first vector.
    /// * `b` - The second vector.
    ///
    /// # Returns
    ///
    /// The euclidean distance between the two vectors.
    pub fn distance(a: &Vector2, b: &Vector2) -> f32 {
        // Calculate the difference between the two vectors and then take its magnitude.
        (a.clone()-b.clone()).magnitude()
    }

    /// Performs a linear interpolation between two vectors.
    ///
    /// # Arguments
    ///
    /// * `a` - The starting vector.
    /// * `b` - The ending vector.
    /// * `t` - The interpolation factor.
    ///
    /// # Returns
    ///
    /// The interpolated vector.
    pub fn lerp(a: &Vector2, b: &Vector2, t: f32) -> Self {
        // Perform a linear interpolation between the two vectors using the interpolation factor.
        // Clamp the interpolation factor between 0 and 1.
        Vector2::lerp_unclamped(a, b, t.clamp(0.0_f32, 1_f32))
    }

    /// Performs a linear interpolation between two vectors without clamping the interpolation factor.
    ///
    /// # Arguments
    ///
    /// * `a` - The starting vector.
    /// * `b` - The ending vector.
    /// * `t` - The interpolation factor.
    ///
    /// # Returns
    ///
    /// The interpolated vector.
    pub fn lerp_unclamped(a: &Vector2, b: &Vector2, t: f32) -> Self {
        // Perform a linear interpolation between the two vectors using the interpolation factor.
        a.clone() * (1_f32 - t) + b.clone() * t
    }

    /// Returns a new vector with the maximum values of the components of two vectors.
    ///
    /// # Arguments
    ///
    /// * `lhs` - The first vector.
    /// * `rhs` - The second vector.
    ///
    /// # Returns
    ///
    /// A new vector with the maximum values of the components of the two input vectors.
    pub fn max(lhs: &Vector2, rhs: &Vector2) -> Self {
        Self {
            x: lhs.x.max(rhs.x),
            y: lhs.y.max(rhs.y)
        }
    }

    /// Returns a new vector with the minimum values of the components of two vectors.
    ///
    /// # Arguments
    ///
    /// * `lhs` - The first vector.
    /// * `rhs` - The second vector.
    ///
    /// # Returns
    ///
    /// A new vector with the minimum values of the components of the two input vectors.
    pub fn min(lhs: &Vector2, rhs: &Vector2) -> Self {
        Self {
            x: lhs.x.min(rhs.x),
            y: lhs.y.min(rhs.y)
        }
    }

    /// Reflects a vector off the surface defined by a normal.
    ///
    /// # Arguments
    ///
    /// * `in_direction` - The incident vector.
    /// * `in_normal` - The normal vector. It has to be normalized.
    ///
    /// # Returns
    ///
    /// The reflected vector.
    pub fn reflect(in_direction: &Vector2, in_normal: &Vector2) -> Self {
        in_direction.clone() - in_normal.clone() * 2_f32 * Vector2::dot(in_direction, in_normal)
    }

    /// Scales a vector by another vector component-wise.
    ///
    /// # Arguments
    ///
    /// * `a` - The first vector.
    /// * `b` - The second vector.
    ///
    /// # Returns
    ///
    /// A new vector that is the result of scaling vector `a` by vector `b`.
    pub fn scale(a: &Vector2, b: &Vector2) -> Self {
        Self {
            x: a.x * b.x,
            y: a.y * b.y
        }
    }

    /// Calculates the cross product of two vectors.
    ///
    /// # Arguments
    ///
    /// * `a` - The first vector.
    /// * `b` - The second vector.
    ///
    /// # Returns
    ///
    /// The cross product of the two vectors.
    pub fn cross(a: &Vector2, b: &Vector2) -> f32 {
        a.x * b.y - a.y * b.x
    }

    /// Calculates the signed angle in degrees between two vectors.
    ///
    /// # Arguments
    ///
    /// * `from` - The starting vector.
    /// * `to` - The ending vector.
    ///
    /// # Returns
    ///
    /// The signed angle in degrees between the two vectors.
    pub fn signed_angle(from: &Vector2, to: &Vector2) -> f32 {
        // Calculate the cross product of the two vectors and then take the arctangent of the result.
        // Convert the result to degrees.
        (Vector2::cross(from, to)).atan2(Vector2::dot(from, to)).to_degrees()
    }
}