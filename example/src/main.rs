use axle2D::App::App;
use axle2D::config::{GlobalConfig, Resolution};

fn main() {
    let global_config = GlobalConfig {
        resolution: Resolution {
            width: 1024,
            height: 576
        },
        title: "Example of axle2D".to_string(),
        gravity: 2_f32,
        fps: 60,
        
    };
}
