use std::ops::{Add, Div, Mul, Sub};

#[derive(Debug, Clone, Copy)]
pub struct Vector2 {
    pub x: f32,
    pub y: f32
}

impl Add for Vector2 {
    type Output = Self;

    fn add(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x + rhs.x,
            y: self.y + rhs.y
        }
    }
}

impl Sub for Vector2 {
    type Output = Self;

    fn sub(self, rhs: Self) -> Self::Output {
        Self {
            x: self.x - rhs.x,
            y: self.y - rhs.y
        }
    }
}

impl Mul<f32> for Vector2 {
    type Output = Self;

    fn mul(self, scalar: f32) -> Self::Output {
        Self {
            x: self.x * scalar,
            y: self.y * scalar
        }
    }
}

impl Div<f32> for Vector2 {
    type Output = Self;

    fn div(self, scalar: f32) -> Self::Output {
        Self {
            x: self.x / scalar,
            y: self.y / scalar
        }
    }
}

//basic implemetations
impl Vector2 {
    pub fn new(x: f32, y: f32) -> Self {
        Self { x, y }
    }
    pub fn down() -> Self {
        Self {
            x: 0.0,
            y: -1.0
        }
    }
    pub fn left() -> Self {
        Self {
            x: -1.0,
            y: 0.0
        }
    }
    pub fn right() -> Self {
        Self {
            x: 1.0,
            y: 0.0
        }
    }
    pub fn up() -> Self {
        Self {
            x: 0.0,
            y: 1.0
        }
    }
    pub fn zero() -> Self {
        Self {
            x: 0.0,
            y: 0.0
        }
    }
    pub fn one() -> Self {
        Self {
            x: 1.0,
            y: 1.0
        }
    }
}

//other more complex implementations
impl Vector2 {
    pub fn sqr_magnitude(&self) -> f32 {
        self.x * self.x + self.y * self.y
    }
    pub fn magnitude(&self) -> f32 {
        self.sqr_magnitude().sqrt()
    }
    pub fn normalized(&self) -> Self {
        let magnitude = self.magnitude();
        Self {
            x: self.x / magnitude,
            y: self.y / magnitude
        }
    }
    pub fn normalize(&mut self) {
        let magnitude = self.magnitude();
        self.x /= magnitude;
        self.y /= magnitude;
    }
    pub fn clamp_magnitude(&mut self, max_magnitude: f32) {
        let f = max_magnitude / self.magnitude();

        self.x *= f;
        self.y *= f;
    }
    pub fn perpendicular(&self) -> Self {
        Self {
            x: -self.y,
            y: self.x
        }
    }
}

//static implementations
impl Vector2 {
    pub fn angle(from: &Vector2, to: &Vector2) -> f32 {
        (Vector2::dot(from, to)/(from.magnitude() * to.magnitude())).acos().to_degrees()
    }
    pub fn dot(lhs: &Vector2, rhs: &Vector2) -> f32 {
        lhs.x * rhs.x + lhs.y * rhs.y
    }
    pub fn distance(a: &Vector2, b: &Vector2) -> f32 {
        (a.clone()-b.clone()).magnitude()
    }
    pub fn lerp(a: &Vector2, b: &Vector2, t: f32) -> Self {
        Vector2::lerp_unclamped(a, b, t.clamp(0.0_f32, 1_f32))
    }
    pub fn lerp_unclamped(a: &Vector2, b: &Vector2, t: f32) -> Self {
        a.clone() * (1_f32 - t) + b.clone() * t
    }
    pub fn max(lhs: &Vector2, rhs: &Vector2) -> Self {
        Self {
            x: lhs.x.max(rhs.x),
            y: lhs.y.max(rhs.y)
        }
    }
    pub fn min(lhs: &Vector2, rhs: &Vector2) -> Self {
        Self {
            x: lhs.x.min(rhs.x),
            y: lhs.y.min(rhs.y)
        }
    }
    pub fn reflect(in_direction: &Vector2, in_normal: &Vector2) -> Self {
        in_direction.clone() - in_normal.clone() * 2_f32 * Vector2::dot(in_direction, in_normal)
    }
    pub fn scale(a: &Vector2, b: &Vector2) -> Self {
        Self {
            x: a.x * b.x,
            y: a.y * b.y
        }
    }
    pub fn cross(a: &Vector2, b: &Vector2) -> f32 {
        a.x * b.y - a.y * b.x
    }
    pub fn signed_angle(from: &Vector2, to: &Vector2) -> f32 {
        (Vector2::cross(from, to)).atan2(Vector2::dot(from, to)).to_degrees()
    }
}