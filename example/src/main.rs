use axle2D::ecs::game_object;
use axle2D::engine::Vector2;

fn main() {
    let lhs = Vector2::new(3_f32, 4_f32);
    let rhs = Vector2::new(5_f32, 6_f32);

    println!("{:?}", Vector2::signed_angle(&lhs, &rhs));
}
