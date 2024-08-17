use crate::engine::Vector2;

/// Struct representing a 2D transformation
///
/// This struct contains the position, rotation, and scale of a 2D object.
#[derive(Debug, Copy, Clone)]
pub struct Transform {
    /// Position of the object
    pub position: Vector2,
    /// Rotation of the object in radians
    pub rotation: f32,
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
            self.rotation.cos() * v.x - self.rotation.sin() * v.y, 
            self.rotation.sin() * v.x + self.rotation.cos() * v.y
        )
    }

    /// Returns the red axis of the transform in world space.
    ///
    /// The red axis represents the x-coordinate of the transform in world space.
    /// This method calculates the vector representing the red axis of the transform,
    /// taking into account the rotation of the transform.
    ///
    /// When a GameObject is rotated, the red axis (represented by the x-coordinate)
    /// changes direction. This method returns the vector representing the red axis
    /// after the rotation is applied.
    ///
    /// If you want to move the GameObject on the x-axis while ignoring rotation,
    /// you can use `Vector2::right()`.
    ///
    /// # Returns
    /// A `Vector2` representing the red axis of the transform in world space.
    pub fn right(&self) -> Vector2 {
        // Calculate the components of the vector representing the red axis
        // after taking into account the rotation of the transform.
        // The x-component is the cosine of the rotation angle,
        // and the y-component is the sine of the rotation angle.
        Vector2::new(self.rotation.cos(), self.rotation.sin())
    }

    /// Returns the The green axis of the transform in world space.
    ///
    /// The green axis represents the y-coordinate of the transform in world space. 
    /// 
    /// This method calculates the vector by rotating the red axis vector by 90 degrees
    /// in a counter-clockwise direction for a 2D coordinate system where the positive Y axis goes up.
    ///
    /// # Returns
    /// A `Vector2` representing the up axis of the transform in world space for a 2D coordinate system where the positive Y axis goes up.
    pub fn up(&self) -> Vector2 {
        // The perpendicular vector is obtained by rotating the red axis vector by 90 degrees
        // in a counter-clockwise direction for a 2D coordinate system where the positive Y axis goes up.
        // So, to obtain the perpendicular vector, we can simply swap the x and y coordinates of the red axis vector.
        self.right().perpendicular()
    }

    /// Sets the rotation of the transform to face towards a target transform's position
    /// so that the right vector is pointing at it.
    ///
    /// # Arguments
    ///
    /// * `target` - The target transform whose position the transform should face towards.
    pub fn look_at(&mut self, target: &Transform) {
        // Calculate the direction from the current position to the target's position.
        let direction_normalized = (target.position - self.position).normalized();
        // Calculate the angle between the current rotation's red axis and the direction.
        let angle_radians = Vector2::signed_angle(&Vector2::right(), &direction_normalized).to_radians();
        // Set the rotation of the transform to the calculated angle.
        self.rotation = angle_radians;
    }

    /// Sets the rotation of the transform to face towards a target'a position
    /// so that the right vector is pointing at it.
    /// 
    /// # Arguments
    ///
    /// * `target` - The target position the transform should face towards.
    pub fn look_at_position(&mut self, target: &Vector2) {
        // Calculate the direction from the current position to the target position.
        let direction_normalized = (target.clone() - self.position).normalized();
        // Calculate the angle between the current rotation's red axis and the direction.
        let angle_radians = Vector2::signed_angle(&Vector2::right(), &direction_normalized).to_radians();
        // Set the rotation of the transform to the calculated angle.
        self.rotation = angle_radians;
    }

    /// Rotates the transform by the given angle in degrees
    ///
    /// # Arguments
    ///
    /// * `angle` - The angle to rotate by in degrees.
    pub fn rotate(&mut self, angle: f32) {
        self.rotation += angle.to_radians()
    }

    /// Rotates the transform around a given point by the specified angle in degrees.
    ///
    /// # Arguments
    ///
    /// * `point` - The point to rotate around.
    /// * `angle` - The angle to rotate by in degrees.
    pub fn rotate_around(&mut self, point: &Vector2, angle: f32) {
        // Calculate the distance from the transform's position to the given point.
        let radius = Vector2::distance(&self.position, point);

        // Calculate the direction vector from the transform's position to the given point.
        let direction = self.position - point.clone();

        // Calculate the signed angle between the positive X-axis and the direction vector.
        let angle_between_zero = Vector2::signed_angle(&Vector2::right(), &direction);

        // Rotate the transform's position around the given point.
        // The angle is calculated by adding the specified angle to the angle between the positive X-axis and the direction vector.
        self.position = Vector2::new(
            point.x + radius * (angle + angle_between_zero).to_radians().cos(),
            point.y + radius * (angle + angle_between_zero).to_radians().sin()
        );
    }

    // TODO: Take into account rotation and scale in this function. Or add other methods in which this is considered
    // TODO: usefull link: https://shorturl.at/c9SCq
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
        point.clone() + self.position
    }

    /// Translates the transform by the given translation vector.
    ///
    /// # Arguments
    ///
    /// * `translation` - The translation vector to apply to the transform.
    pub fn translate(&mut self, translation: &Vector2) {
        // Add the translation vector to the position of the transform.
        self.position = self.position + translation.clone();
    }
}

