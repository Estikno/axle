use eyre::Result;
use std::ops::Mul;

use crate::vector::Vector2;
use crate::custom_errors::CustomErrors;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rot2 {
    pub cos: f32,
    pub sin: f32
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
            rhs.x * self.sin + rhs.y * self.cos
        )
    }
}

impl Rot2 {
    /// No rotation.
    pub const IDENTITY: Self = Self { cos: 1.0, sin: 0.0 };
    /// A rotation of π radians.
    pub const PI: Self = Self { cos: -1.0, sin: 0.0 };
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
        let rot = Self { sin: sine, cos: cosine };
        assert!(!rot.is_normalized(), "The given sine and cosine do not represent a valid rotation");
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
}
