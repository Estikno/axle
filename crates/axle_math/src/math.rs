use std::f32::{self, consts::PI};

/// The constant to convert degrees to radians.
pub const DEG2RAD: f32 = PI / 180.0_f32;

/// The constant to convert radians to degrees.
pub const RAD2DEG: f32 = 180.0_f32 / PI;

/// Tests if two floats are equal within a certain precision.
///
/// Two floats are considered equal if their absolute difference is less than or equal to
/// `epsilon`.
///
/// # Arguments
///
/// * `a` - The first float
/// * `b` - The second float
/// * `epsilon` - The maximum difference between the two floats
///
/// # Returns
///
/// `true` if the two floats are aproximately equal, `false` otherwise
pub fn aproximately(a: f32, b: f32, epsilon: f32) -> bool {
    (a - b).abs() <= epsilon
}

/// Performs a linear interpolation between two floats without clamping the interpolation factor.
///
/// # Arguments
///
/// * `a` - The starting float
/// * `b` - The ending float
/// * `t` - The interpolation factor
///
/// # Returns
///
/// The interpolated float
pub fn lerp_unclamped(a: f32, b: f32, t: f32) -> f32 {
    (1.0 - t) * a + t * b
}

/// Performs a linear interpolation between two floats without clamping the interpolation factor.
///
/// # Arguments
///
/// * `a` - The starting float
/// * `b` - The ending float
/// * `t` - The interpolation factor
///
/// # Returns
///
/// The interpolated float
pub fn lerp(a: f32, b: f32, t: f32) -> f32 {
    lerp_unclamped(a, b, t.clamp(0.0, 1.0))
}

/// Performs a linear interpolation between two floats in the range of 0 to 360.
///
/// This method returns the shortest path between the specified angles. 
/// This method wraps around values that are outside the range [-180, 180]. For example, LerpAngle(1.0f, 190.0f, 1.0f) returns -170.0f. 
/// To find the longest path use Lerp.
/// 
/// # Arguments
///
/// * `a` - The starting float
/// * `b` - The ending float
/// * `t` - The interpolation factor
///
/// # Returns
///
/// The interpolated float
pub fn lerp_angle(a: f32, b: f32, t: f32) -> f32 {
    let delta = (b - a) % 360.0;
    let short_angle = if delta > 180.0 { delta - 360.0 } else { delta };
    a + short_angle * t
}

/// Moves a float towards a target value by a maximum amount.
///
/// This function is similar to `Lerp` except that this function ensures the rate of change never exceeds maxDelta and 
/// that the current value is never greater than the target value. Negative values of maxDelta pushes the value away from target.
///
/// # Arguments
///
/// * `current` - The current float
/// * `target` - The target float
/// * `max_delta` - The maximum amount to move the float
///
/// # Returns
///
/// The updated float
pub fn move_towards(current: f32, target: f32, max_delta: f32) -> f32 {
    if (target - current).abs() <= max_delta {
        return target;
    }

    current + (target - current).signum() * max_delta
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_aproximately() {
        assert!(aproximately(0.0, 0.0, 0.0));
        assert!(aproximately(0.0, 0.001, 0.01));
        assert!(aproximately(1.0, 1.0, 0.0));
        assert!(aproximately(1.0, 1.01, 0.01));
        assert!(!aproximately(0.0, 0.01, 0.001));
        assert!(!aproximately(1.0, 1.1, 0.01));
    }

    #[test]
    fn test_lerp_unclamped() {
        assert_eq!(lerp_unclamped(0.0, 10.0, 0.5), 5.0);
        assert_eq!(lerp_unclamped(0.0, 10.0, 1.0), 10.0);
        assert_eq!(lerp_unclamped(0.0, 10.0, -1.0), -10.0);
    }

    #[test]
    fn test_lerp() {
        assert_eq!(lerp(0.0, 10.0, 0.5), 5.0);
        assert_eq!(lerp(0.0, 10.0, 1.0), 10.0);
        assert_eq!(lerp(0.0, 10.0, -1.0), 0.0);
    }

    #[test]
    fn test_lerp_angle() {
        assert_eq!(lerp_angle(0.0, 360.0, 0.5), 0.0);
        assert_eq!(lerp_angle(1.0, 190.0, 1.0), -170.0);
    }

    #[test]
    fn test_move_towards() {
        assert_eq!(move_towards(10.0, 15.0, 5.0), 15.0);
        assert_eq!(move_towards(10.0, 5.0, 5.0), 5.0);
        assert_eq!(move_towards(10.0, 5.0, 2.0), 8.0);
    }
}