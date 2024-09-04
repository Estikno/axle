use std::f32::{self, consts::PI};

pub const DEG2RAD: f32 = PI / 180.0_f32;
pub const RAD2DEG: f32 = 180.0_f32 / PI;

pub fn aproximately(a: f32, b: f32) -> bool {
    a >= b - f32::EPSILON && a <= b + f32::EPSILON
}

pub fn lerp_unclamped(a: f32, b: f32, t: f32) -> f32 {
    (1.0 - t) * a + t * b
}

pub fn lerp(a: f32, b: f32, t: f32) -> f32 {
    lerp_unclamped(a, b, t.clamp(0.0, 1.0))
}

pub fn lerp_angle(a: f32, b: f32, t: f32) -> f32 {
    let delta = (b - a) % 360.0;
    let short_angle = if delta > 180.0 { delta - 360.0 } else { delta };
    a + short_angle * t
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_lerping_angle() {
        assert_eq!(lerp_angle(0.0, 360.0, 0.5), 0.0);
        assert_eq!(lerp_angle(1.0, 190.0, 1.0), -170.0);
    }
}