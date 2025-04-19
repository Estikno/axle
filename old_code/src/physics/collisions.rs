use axle_math::vector::Vector2;
use core::f32;

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
pub fn intersect_circles(
    pos_a: Vector2,
    radius_a: f32,
    pos_b: Vector2,
    radius_b: f32,
) -> Option<(Vector2, f32)> {
    // Calculate the distance between the centers of the circles
    let distance = Vector2::distance(&pos_a, &pos_b);

    // Calculate the combined radius of the circles
    let radii = radius_a + radius_b;

    // If the distance between the centers is greater than or equal to the combined radius, the circles do not intersect
    if distance >= radii {
        return None;
    }

    // Calculate the normal vector of the collision, which points from one circle to the other
    let normal = (pos_b - pos_a).normalized();

    // Calculate the depth of the collision, which is the distance between the circles at the point of collision
    let depth = radii - distance;

    // Return the collision normal and depth as a tuple
    Some((normal, depth))
}

// vertices have to be transformed (rotated and in world position)
pub fn intersect_polygons(vertices_a: &[Vector2], vertices_b: &[Vector2]) -> bool {
    fn project_vertices(vertices: &[Vector2], axis: &Vector2, min: &mut f32, max: &mut f32) {
        *min = f32::MAX;
        *max = f32::MIN;

        for vertex in vertices {
            let projection = Vector2::scalar_projection(vertex, axis);
            if projection < *min {
                *min = projection;
            }
            if projection > *max {
                *max = projection;
            }
        }
    }

    for i in 0..vertices_a.len() {
        let va = vertices_a[i];
        let vb = vertices_a[(i + 1) % vertices_a.len()];
        let edge = vb - va;
        let axis = edge.perpendicular();

        let mut min_a = 0.0;
        let mut max_a = 0.0;
        project_vertices(vertices_a, &axis, &mut min_a, &mut max_a);

        let mut min_b = 0.0;
        let mut max_b = 0.0;
        project_vertices(vertices_b, &axis, &mut min_b, &mut max_b);

        if min_a >= max_b || min_b >= max_a {
            return false; // No collision if projections don't overlap on this axis
        }
    }

    for i in 0..vertices_b.len() {
        let va = vertices_b[i];
        let vb = vertices_b[(i + 1) % vertices_b.len()];
        let edge = vb - va;
        let axis = edge.perpendicular();

        let mut min_a = 0.0;
        let mut max_a = 0.0;
        project_vertices(vertices_a, &axis, &mut min_a, &mut max_a);

        let mut min_b = 0.0;
        let mut max_b = 0.0;
        project_vertices(vertices_b, &axis, &mut min_b, &mut max_b);

        if min_a >= max_b || min_b >= max_a {
            return false; // No collision if projections don't overlap on this axis
        }
    }

    true
}

