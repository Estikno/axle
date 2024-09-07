use eyre::Result;

use crate::vector::Vector2;
use crate::custom_errors::CustomErrors;

#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Rot2 {
    pub cos: f32,
    pub sin: f32
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
    
    pub fn radians(radians: f32) -> Self {
        let (sin, cos) = radians.sin_cos();
        Self::from_sin_cos(sin, cos)
    }
    
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

    pub fn as_radians(self) -> f32 {
        f32::atan2(self.sin, self.cos)
    }

    pub fn as_degrees(self) -> f32 {
        self.as_radians().to_degrees()
    }

    pub fn sin_cos(&self) -> (f32, f32) {
        (self.sin, self.cos)
    }

    pub fn sqr_magnitude(&self) -> f32 {
        Vector2::new(self.cos, self.sin).sqr_magnitude()
    }

    pub fn magnitude(&self) -> f32 {
       self.sqr_magnitude().sqrt() 
    }

    pub fn magnitude_recip(&self) -> f32 {
        1.0 / self.magnitude()
    }

    pub fn try_normalize(&mut self) -> Result<()> {
        let recip = self.magnitude_recip();
        if recip.is_finite() && recip > 0.0 {
           *self = Self::from_sin_cos(self.sin * recip, self.cos * recip);
           Ok(()) 
        }
        else {
            Err(CustomErrors::CouldNotNormalize("sine and cosine").into())
        }
    }

    pub fn is_normalized(self) -> bool {
        // The allowed length is 1 +/- 1e-4, so the largest allowed
        // squared length is (1 + 1e-4)^2 = 1.00020001, which makes
        // the threshold for the squared length approximately 2e-4.
        (self.sqr_magnitude() - 1.0).abs() <= 2e-4
    }
}