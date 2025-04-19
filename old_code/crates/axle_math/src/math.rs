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

/// Loops the value t, so that it is never larger than length and never smaller than 0.
///
/// This is similar to the modulo operator but it works with floating point numbers.
/// For example, using 3.0 for t and 2.5 for length, the result would be 0.5. With t = 5 and length = 2.5, the result would be
/// 0.0. Note, however, that the behaviour is not defined for negative numbers as it is for the modulo operator.
///
/// # Arguments
///
/// * `t` - The value to loop
/// * `lenght` - The length of the loop
///
/// # Returns
///
/// The updated float
pub fn repeat(t: f32, lenght: f32) -> f32 {
    (t - (t / lenght).floor() * lenght).clamp(0.0, lenght)
}

/// Calculates the shortest difference between two angles.
///
/// # Arguments
///
/// * `current` - The current angle in degrees
/// * `target` - The target angle in degrees
///
/// # Returns
///
/// A value between -179 and 180, in degrees.
pub fn delta_angle(current: f32, target: f32) -> f32 {
    let delta = repeat(target - current, 360.0);
    if delta > 180.0 {
        delta - 360.0
    } else {
        delta
    }
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
    a + (b - a) * t
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
    let mut delta = repeat(b - a, 360.0);

    if delta > 180.0 {
        delta -= 360.0;
    } else if delta < -180.0 {
        delta += 360.0;
    }

    (a + delta * t.clamp(0.0, 1.0)) % 360.0
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

/// Same as `MoveTowards` but makes sure the values interpolate correctly when they wrap around 360 degrees.
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
pub fn move_towards_angle(current: f32, target: f32, max_delta: f32) -> f32 {
    let delta_angle = delta_angle(current, target);
    let clamped_delta = delta_angle.clamp(-max_delta, max_delta);

    if (delta_angle - clamped_delta).abs() < f32::EPSILON {
        return target;
    }

    current + clamped_delta
}

#[cfg(test)]
mod tests {
    use super::*;
    use assert_approx_eq::assert_approx_eq;

    #[test]
    fn test_aproximately() {
        assert!(aproximately(1.0, 1.0, 0.0001));
        assert!(!aproximately(1.0, 1.1, 0.0001));
        assert!(aproximately(1.0, 1.1, 0.2));
    }

    #[test]
    fn test_repeat() {
        assert_eq!(repeat(3.0, 2.5), 0.5);
        assert_eq!(repeat(5.0, 2.5), 0.0);
        assert_eq!(repeat(-1.0, 3.0), 2.0); // Note: Behavior with negative numbers is not specified
    }

    #[test]
    fn test_delta_angle() {
        assert_eq!(delta_angle(0.0, 90.0), 90.0);
        assert_eq!(delta_angle(0.0, 270.0), -90.0);
        assert_eq!(delta_angle(350.0, 10.0), 20.0);
        assert_eq!(delta_angle(10.0, 350.0), -20.0);
    }

    #[test]
    fn test_lerp_unclamped() {
        assert_eq!(lerp_unclamped(0.0, 10.0, 0.5), 5.0);
        assert_eq!(lerp_unclamped(10.0, 20.0, 0.0), 10.0);
        assert_eq!(lerp_unclamped(10.0, 20.0, 1.0), 20.0);
    }

    #[test]
    fn test_lerp() {
        assert_eq!(lerp(0.0, 10.0, 0.5), 5.0);
        assert_eq!(lerp(10.0, 20.0, -0.5), 10.0); // Clamped to 0
        assert_eq!(lerp(10.0, 20.0, 1.5), 20.0); // Clamped to 1
    }

    #[test]
    fn test_lerp_angle() {
        assert_eq!(lerp_angle(0.0, 90.0, 0.5), 45.0);
        assert_eq!(lerp_angle(0.0, 270.0, 0.5), -45.0);
        assert_eq!(lerp_angle(350.0, 10.0, 0.5), 0.0);
    }

    #[test]
    fn test_move_towards() {
        assert_eq!(move_towards(0.0, 10.0, 5.0), 5.0);
        assert_eq!(move_towards(0.0, 10.0, 15.0), 10.0);
        assert_eq!(move_towards(10.0, 0.0, 5.0), 5.0);
        assert_eq!(move_towards(10.0, 0.0, 15.0), 0.0);
    }

    #[test]
    fn test_move_towards_angle() {
        assert_approx_eq!(move_towards_angle(0.0, 90.0, 45.0), 45.0);
        assert_approx_eq!(move_towards_angle(0.0, 90.0, 100.0), 90.0);
        assert_approx_eq!(move_towards_angle(350.0, 10.0, 20.0), 10.0);
        assert_approx_eq!(move_towards_angle(10.0, 350.0, 20.0), 350.0); // Wraps around
    }
}
