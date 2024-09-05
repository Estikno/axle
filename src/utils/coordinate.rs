use axle_math::vector::Vector2;

pub fn convert_point_y(y: f32) -> f32 {
    // !Arbitrary window height just to make sure the coordinate system is correct
    -y + 576.0
}

pub fn convert_vector_y(v: &Vector2) -> Vector2 {
    Vector2::new(v.x, convert_point_y(v.y))
}