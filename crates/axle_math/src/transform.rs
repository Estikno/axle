use crate::rotation::Rot2;
use crate::vector::Vector2;

#[derive(Debug, Clone)]
pub struct Transform {
    pub position: Vector2,
    pub rotation: Rot2,
    // TODO: Add scale component
}

impl Transform {
    pub const IDENTITY: Self = Self {
        position: Vector2::ZERO,
        rotation: Rot2::IDENTITY,
    };

    pub fn from_xy(x: f32, y: f32) -> Self {
        Self::from_position(Vector2::new(x, y))
    }

    pub fn from_position(position: Vector2) -> Self {
        Self {
            position,
            ..Self::IDENTITY
        }
    }

    pub fn from_rotation(rotation: Rot2) -> Self {
        Self {
            rotation,
            ..Self::IDENTITY
        }
    }
}

impl Transform {
    pub fn right(&self) -> Vector2 {
        Vector2::new(self.rotation.cos, self.rotation.sin)
    }

    pub fn up(&self) -> Vector2 {
        Vector2::new(-self.rotation.sin, self.rotation.cos)
    }

    /// Sets the rotation of the transform to face towards a target transform's position
    /// so that the right vector is pointing at it.
    ///
    /// # Arguments
    ///
    /// * `target` - The target transform whose position the transform should face towards.
    pub fn look_at(&mut self, target: &Transform) {
        self.look_at_position(&target.position)
    }

    /// Same as `look_at`, but takes a position instead of a transform.
    ///
    /// # Arguments
    ///
    /// * `target` - The target position the transform should face towards.
    pub fn look_at_position(&mut self, target: &Vector2) {
        // Calculate the direction from the current position to the target position.
        let direction_normalized = (*target - self.position).normalized();
        // Calculate the angle between the current rotation's red axis and the direction.
        let angle_radians = Vector2::signed_angle(&Vector2::RIGHT, &direction_normalized);
        // Set the rotation of the transform to the calculated angle.
        self.rotation = Rot2::radians(angle_radians);
    }

    /// Rotates the transform by the given angle in radians
    ///
    /// # Arguments
    ///
    /// * `angle` - The angle to rotate by in radians.
    pub fn rotate(&mut self, angle: f32) {
        self.rotation = Rot2::radians(self.rotation.as_radians() + angle);
    }

    /// Rotates the transform around a given point by the specified angle in radians.
    ///
    /// # Arguments
    ///
    /// * `point` - The point to rotate around.
    /// * `angle` - The angle to rotate by in degrees.
    pub fn rotate_around(&mut self, point: &Vector2, angle: f32) {
        // Calculate the distance from the transform's position to the given point.
        let radius = Vector2::distance(&self.position, point);

        // Calculate the direction vector from the transform's position to the given point.
        let direction = self.position - *point;

        // Calculate the signed angle between the positive X-axis and the direction vector.
        let angle_between_zero = Vector2::signed_angle(&Vector2::RIGHT, &direction);

        // Rotate the transform's position around the given point.
        // The angle is calculated by adding the specified angle to the angle between the positive X-axis and the direction vector.
        self.position = Vector2::new(
            point.x + radius * (angle + angle_between_zero).cos(),
            point.y + radius * (angle + angle_between_zero).sin(),
        );
    }

    // TODO: Take into account rotation and scale in this function. Or add other methods in which this is considered
    // TODO: usefull link: https://shorturl.at/c9SCq
    //
    /// Transforms a point from local space to world space.
    /// This functions does not take into account neither rotation nor scale
    ///
    /// # Arguments
    ///
    /// * `point` - The point to transform
    ///
    /// # Returns
    ///
    /// A `Vector2` representing the transformed point
    pub fn transform_point(&self, point: &Vector2) -> Vector2 {
        // Add the the point to the position of the transform to get the world position of the point
        *point + self.position
    }

    /// Translates the transform by the given translation vector.
    ///
    /// # Arguments
    ///
    /// * `translation` - The translation vector to apply to the transform.
    pub fn translate(&mut self, translation: &Vector2) {
        // Add the translation vector to the position of the transform.
        self.position = self.position + *translation;
    }

    /// Rotates a vector in 2D by the transform's rotation.
    /// The resultant vector is represented in local space, meaning it is not affected by the transform's position or scale.
    ///
    /// # Arguments
    ///
    /// * `v` - The vector to be rotated.
    ///
    /// # Returns
    ///
    /// A new vector that is the result of rotating `v` by the transform's rotation.
    pub fn transform_vector(&self, v: &Vector2) -> Vector2 {
        // Formula for rotating a vector in 2D
        // x2=cosβx1−sinβy1
        // y2=sinβx1+cosβy1
        // https://matthew-brett.github.io/teaching/rotation_2d.html

        Vector2::new(
            self.rotation.cos * v.x - self.rotation.sin * v.y,
            self.rotation.sin * v.x + self.rotation.cos * v.y,
        )
    }
}
