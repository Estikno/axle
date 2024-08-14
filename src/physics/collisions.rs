use crate::engine::Vector2;

pub fn intersect_circles(posA: Vector2, radiusA: f32, posB: Vector2, radiusB: f32) -> Option<(Vector2, f32)> {
    let distance = Vector2::distance(&posA, &posB);
    let radii = radiusA + radiusB;

    if distance >= radii {
        return None;
    }

    let normal = (posA - posB).normalized();
    let depth = radii - distance;

    return Some((normal, depth));
}