use std::rc::Rc;

use axle2D::app::App;
use axle2D::config::{GlobalConfig, Resolution};

//sdl2 keys
use sdl2::keyboard::Keycode;
use sdl2::mouse::MouseButton;

fn main() {
    let global_config = GlobalConfig::new(Resolution { width: 1024, height: 576}, String::from("Axle Tests"), 4_f32, 60);
    let rc_global_config = Rc::new(global_config);

    let mut app = App::new(rc_global_config);
        
    loop {
        if app.update() {
            break;
        }

        if app.input.is_key_pressed(Keycode::W) {
            println!("Mouse left pressed");
        }

        if app.input.is_key_released(Keycode::W) {
            println!("Mouse left released");
        }

        if app.input.is_key_held(Keycode::W) {
            println!("Mouse left holding");
        }
    }
}

