use crate::engine::Vector2;

/// Calculates the intersection point between two circles if they intersect.
///
/// # Arguments
///
/// * `pos_a` - The center position of the first circle.
/// * `radius_a` - The radius of the first circle.
/// * `pos_b` - The center position of the second circle.
/// * `radius_b` - The radius of the second circle.
///
/// # Returns
///
/// If the circles intersect, returns the collision normal and depth as a tuple. Otherwise, returns None.
pub fn intersect_circles(pos_a: Vector2, radius_a: f32, pos_b: Vector2, radius_b: f32) -> Option<(Vector2, f32)> {
    // Calculate the distance between the centers of the circles
    let distance = Vector2::distance(&pos_a, &pos_b);

    // Calculate the combined radius of the circles
    let radii = radius_a + radius_b;

    // If the distance between the centers is greater than or equal to the combined radius, the circles do not intersect
    if distance >= radii {
        return None;
    }

    // Calculate the normal vector of the collision, which points from one circle to the other
    let normal = (pos_a - pos_b).normalized();

    // Calculate the depth of the collision, which is the distance between the circles at the point of collision
    let depth = radii - distance;

    // Return the collision normal and depth as a tuple
    Some((normal, depth))
}
