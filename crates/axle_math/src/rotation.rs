use eyre::Result;
use std::ops::Mul;

use crate::custom_errors::CustomErrors;
use crate::math::lerp;
use crate::vector::Vector2;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rot2 {
    pub cos: f32,
    pub sin: f32,
}

impl Mul for Rot2 {
    type Output = Self;

    fn mul(self, rhs: Self) -> Self::Output {
        Self {
            cos: self.cos * rhs.cos - self.sin * rhs.sin,
            sin: self.sin * rhs.cos + self.cos * rhs.sin,
        }
    }
}

impl Mul<Vector2> for Rot2 {
    type Output = Vector2;

    fn mul(self, rhs: Vector2) -> Self::Output {
        Vector2::new(
            rhs.x * self.cos - rhs.y * self.sin,
            rhs.x * self.sin + rhs.y * self.cos,
        )
    }
}

impl Rot2 {
    /// No rotation.
    pub const IDENTITY: Self = Self { cos: 1.0, sin: 0.0 };
    /// A rotation of π radians.
    pub const PI: Self = Self {
        cos: -1.0,
        sin: 0.0,
    };
    /// A counterclockwise rotation of π/2 radians.
    pub const FRAC_PI_2: Self = Self { cos: 0.0, sin: 1.0 };
    /// A counterclockwise rotation of π/3 radians.
    pub const FRAC_PI_3: Self = Self {
        cos: 0.5,
        sin: 0.866_025_4,
    };
    /// A counterclockwise rotation of π/4 radians.
    pub const FRAC_PI_4: Self = Self {
        cos: core::f32::consts::FRAC_1_SQRT_2,
        sin: core::f32::consts::FRAC_1_SQRT_2,
    };
    /// A counterclockwise rotation of π/6 radians.
    pub const FRAC_PI_6: Self = Self {
        cos: 0.866_025_4,
        sin: 0.5,
    };
    /// A counterclockwise rotation of π/8 radians.
    pub const FRAC_PI_8: Self = Self {
        cos: 0.923_879_5,
        sin: 0.382_683_43,
    };

    /// Create a new `Rot2` representing a counterclockwise rotation of `radians` radians.
    ///
    /// # Panics
    ///
    /// If the given `radians` value does not represent a valid rotation (i.e., it is a NaN or infinite value).
    pub fn radians(radians: f32) -> Self {
        let (sin, cos) = radians.sin_cos();
        Self::from_sin_cos(sin, cos)
    }

    /// Create a new `Rot2` representing a counterclockwise rotation of `degrees` degrees.
    ///
    /// # Panics
    ///
    /// If the given `degrees` value does not represent a valid rotation (i.e., it is a NaN or infinite value).
    pub fn degrees(degrees: f32) -> Self {
        Self::radians(degrees.to_radians())
    }

    /// Create a new `Rot2` from a sine and cosine.
    ///
    /// # Panics
    ///
    /// If the given sine and cosine do not represent a valid rotation (i.e., they are not normalized).
    pub fn from_sin_cos(sine: f32, cosine: f32) -> Self {
        let rot = Self {
            sin: sine,
            cos: cosine,
        };
        assert!(
            rot.is_normalized(),
            "The given sine and cosine do not represent a valid rotation"
        );
        rot
    }

    /// Returns the angle in radians that this rotation represents.
    ///
    /// # Returns
    ///
    /// The angle in radians that this rotation represents.
    pub fn as_radians(self) -> f32 {
        f32::atan2(self.sin, self.cos)
    }

    /// Returns the angle in degrees that this rotation represents.
    ///
    /// # Returns
    ///
    /// The angle in degrees that this rotation represents.
    pub fn as_degrees(self) -> f32 {
        self.as_radians().to_degrees()
    }

    /// Returns the sine and cosine of the rotation as a tuple.
    ///
    /// # Returns
    ///
    /// A tuple containing the sine and cosine of the rotation.
    pub fn sin_cos(&self) -> (f32, f32) {
        (self.sin, self.cos)
    }
    /// Returns the squared magnitude of the rotation.
    ///
    /// # Returns
    ///
    /// The squared magnitude of the rotation.
    pub fn sqr_magnitude(&self) -> f32 {
        Vector2::new(self.cos, self.sin).sqr_magnitude()
    }

    /// Returns the magnitude of the rotation.
    ///
    /// # Returns
    ///
    /// The magnitude of the rotation.
    pub fn magnitude(&self) -> f32 {
        self.sqr_magnitude().sqrt()
    }

    /// Returns the reciprocal of the magnitude of the rotation.
    ///
    /// # Returns
    ///
    /// The reciprocal of the magnitude of the rotation.
    pub fn magnitude_recip(&self) -> f32 {
        1.0 / self.magnitude()
    }

    /// Modifies the current rotation to have a magnitude of 1.
    ///
    /// # Panics
    ///
    /// This function panics if the current rotation has a magnitude of 0 or one of its components it's either infinite or NaN.
    pub fn normalize(&mut self) {
        self.try_normalize().unwrap()
    }

    /// Same as `normalize` but returns a result with a `CustomErrors::CouldNotNormalize` error if it fails instead of panicking.
    ///
    /// # Errors
    ///
    /// If the current rotation has a magnitude of 0 or one of its components it's either infinite or NaN, an error is returned.
    ///
    /// # Returns
    ///
    /// A result that contains nothing if succeeds or an error if it fails.
    pub fn try_normalize(&mut self) -> Result<()> {
        let recip = self.magnitude_recip();

        // If the magnitude is 0, infinite or NaN, return an error
        if recip.is_nan() || recip.is_infinite() || recip == 0.0 {
            return Err(CustomErrors::CouldNotNormalize("sine and cosine").into());
        }

        // Multiply the sine and cosine of the rotation by the reciprocal of the magnitude.
        *self = Self::from_sin_cos(self.sin * recip, self.cos * recip);
        Ok(())
    }

    /// Returns `true` if the rotation is finite (i.e., not NaN or infinite) and `false` otherwise.
    ///
    /// # Returns
    ///
    /// `true` if the rotation is finite, `false` otherwise.
    pub fn is_finite(&self) -> bool {
        self.sin.is_finite() && self.cos.is_finite()
    }

    /// Returns `true` if the rotation is NaN (Not a Number) and `false` otherwise.
    ///
    /// # Returns
    ///
    /// `true` if the rotation is NaN, `false` otherwise.
    pub fn is_nan(&self) -> bool {
        self.sin.is_nan() || self.cos.is_nan()
    }

    /// Returns `true` if the rotation is normalized (i.e., its magnitude is 1) and `false` otherwise.
    ///
    /// # Returns
    ///
    /// `true` if the rotation is normalized, `false` otherwise.
    pub fn is_normalized(self) -> bool {
        // The allowed length is 1 +/- 1e-4, so the largest allowed
        // squared length is (1 + 1e-4)^2 = 1.00020001, which makes
        // the threshold for the squared length approximately 2e-4.
        (self.sqr_magnitude() - 1.0).abs() <= 2e-4
    }

    /// Returns `true` if the rotation is very close to the identity (i.e., the angle is very close to 0) and `false` otherwise.
    ///
    /// # Returns
    ///
    /// `true` if the rotation is very close to the identity, `false` otherwise.
    pub fn is_near_identity(&self) -> bool {
        let threshold_angle_sin = 0.000_049_692_047; // let threshold_angle = 0.002_847_144_6;
        self.cos > 0.0 && self.sin.abs() < threshold_angle_sin
    }

    /// Returns the angle between two rotations in radians.
    ///
    /// # Arguments
    ///
    /// * `other` - The other rotation to calculate the angle to.
    ///
    /// # Returns
    ///
    /// The angle between the two rotations in radians.
    pub fn angle_between(self, other: Self) -> f32 {
        // Correct error, implement multiplications
        (other * self.inverse()).as_radians()
    }

    /// Returns the inverse of the rotation.
    ///
    /// # Returns
    ///
    /// The inverse of the rotation.
    pub fn inverse(self) -> Self {
        Self {
            cos: self.cos,
            sin: -self.sin,
        }
    }

    /// Performs a linear interpolation between `self` and `rhs` based on
    /// the value `s`, and normalizes the rotation afterwards.
    ///
    /// When `s == 0.0`, the result will be equal to `self`.
    /// When `s == 1.0`, the result will be equal to `rhs`.
    ///
    /// This is slightly more efficient than [`slerp`](Self::slerp), and produces a similar result
    /// when the difference between the two rotations is small. At larger differences,
    /// the result resembles a kind of ease-in-out effect.
    ///
    /// If you would like the angular velocity to remain constant, consider using [`slerp`](Self::slerp) instead.
    ///
    /// # Details
    ///
    /// `nlerp` corresponds to computing an angle for a point at position `s` on a line drawn
    /// between the endpoints of the arc formed by `self` and `rhs` on a unit circle,
    /// and normalizing the result afterwards.
    ///
    /// Note that if the angles are opposite like 0 and π, the line will pass through the origin,
    /// and the resulting angle will always be either `self` or `rhs` depending on `s`.
    /// If `s` happens to be `0.5` in this case, a valid rotation cannot be computed, and `self`
    /// will be returned as a fallback.
    ///
    pub fn nlerp(self, end: Self, s: f32) -> Self {
        let mut result = Self {
            sin: lerp(self.sin, end.sin, s),
            cos: lerp(self.cos, end.cos, s),
        };

        match result.try_normalize() {
            Ok(_) => result,
            Err(_) => self,
        }
    }

    /// Performs a spherical linear interpolation between `self` and `end`
    /// based on the value `s`.
    ///
    /// This corresponds to interpolating between the two angles at a constant angular velocity.
    ///
    /// When `s == 0.0`, the result will be equal to `self`.
    /// When `s == 1.0`, the result will be equal to `rhs`.
    ///
    /// If you would like the rotation to have a kind of ease-in-out effect, consider
    /// using the slightly more efficient [`nlerp`](Self::nlerp) instead.
    pub fn slerp(self, end: Self, s: f32) -> Self {
        self * Self::radians(self.angle_between(end) * s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use assert_approx_eq::assert_approx_eq;

    #[test]
    fn test_mul_rot2() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_4;
        let result = rot1 * rot2;
        assert_approx_eq!(result.cos, 0.0, 1e-5);
        assert_approx_eq!(result.sin, 1.0, 1e-5);
    }

    #[test]
    fn test_mul_vector2() {
        let rot = Rot2::FRAC_PI_4;
        let vec = Vector2::new(1.0, 0.0);
        let result = rot * vec;
        assert_approx_eq!(result.x, 0.70710677, 1e-5);
        assert_approx_eq!(result.y, 0.70710677, 1e-5);
    }

    #[test]
    fn test_identity_rotation() {
        let identity = Rot2::IDENTITY;
        let vec = Vector2::new(1.0, 1.0);
        let result = identity * vec;
        assert_eq!(result, vec);
    }

    #[test]
    fn test_inverse_rotation() {
        let rot = Rot2::FRAC_PI_4;
        let inverse = rot.inverse();
        let result = rot * inverse;
        assert_approx_eq!(result.cos, 1.0, 1e-5);
        assert_approx_eq!(result.sin, 0.0, 1e-5);
    }

    #[test]
    fn test_rotation_magnitude() {
        let rot = Rot2::FRAC_PI_4;
        assert_approx_eq!(rot.magnitude(), 1.0, 1e-5);
    }

    #[test]
    #[should_panic(expected = "The given sine and cosine do not represent a valid rotation")]
    fn test_invalid_from_sin_cos() {
        Rot2::from_sin_cos(1.0, 1.0);
    }

    #[test]
    fn test_angle_between() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let angle = rot1.angle_between(rot2);
        assert_approx_eq!(angle, std::f32::consts::FRAC_PI_4, 1e-5);
    }

    #[test]
    fn test_radians_to_degrees() {
        let rot = Rot2::FRAC_PI_4;
        let degrees = rot.as_degrees();
        assert_approx_eq!(degrees, 45.0, 1e-5);
    }

    #[test]
    fn test_degrees_to_radians() {
        let rot = Rot2::degrees(45.0);
        let radians = rot.as_radians();
        assert_approx_eq!(radians, std::f32::consts::FRAC_PI_4, 1e-5);
    }

    #[test]
    fn test_normalize_rotation() {
        let mut rot = Rot2 { cos: 2.0, sin: 2.0 };
        rot.normalize();
        assert_approx_eq!(rot.magnitude(), 1.0, 1e-5);
    }

    #[test]
    fn test_nlerp_zero_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.nlerp(rot2, 0.0);
        assert_approx_eq!(result.sin, rot1.sin, 1e-5);
        assert_approx_eq!(result.cos, rot1.cos, 1e-5);
    }

    #[test]
    fn test_nlerp_one_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.nlerp(rot2, 1.0);
        assert_approx_eq!(result.sin, rot2.sin, 1e-5);
        assert_approx_eq!(result.cos, rot2.cos, 1e-5);
    }

    #[test]
    fn test_nlerp_half_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.nlerp(rot2, 0.5);
        assert_approx_eq!(result.cos, 0.38268343, 1e-5);
        assert_approx_eq!(result.sin, 0.9238795, 1e-5);
    }

    #[test]
    fn test_slerp_zero_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.slerp(rot2, 0.0);
        assert_approx_eq!(result.sin, rot1.sin, 1e-5);
        assert_approx_eq!(result.cos, rot1.cos, 1e-5);
    }

    #[test]
    fn test_slerp_one_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.slerp(rot2, 1.0);
        assert_approx_eq!(result.sin, rot2.sin, 1e-5);
        assert_approx_eq!(result.cos, rot2.cos, 1e-5);
    }

    #[test]
    fn test_slerp_half_s() {
        let rot1 = Rot2::FRAC_PI_4;
        let rot2 = Rot2::FRAC_PI_2;
        let result = rot1.slerp(rot2, 0.5);
        assert_approx_eq!(result.cos, 0.38268342, 1e-5);
        assert_approx_eq!(result.sin, 0.9238795, 1e-5);
    }
}
