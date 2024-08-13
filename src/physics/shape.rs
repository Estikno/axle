#[derive(Clone)]
pub enum Shape {
    Rectangle { width: f32, height: f32 },
    Circle { radius: f32 }
}

impl Shape {
    pub fn get_area(&self) -> f32 {
        match self {
            Shape::Rectangle { width, height } => width * height,
            Shape::Circle { radius } => std::f32::consts::PI * radius * radius
        }
    }
}