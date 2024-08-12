use std::rc::Rc;

use axle2D::app::App;
use axle2D::config::{GlobalConfig, Resolution};

use axle2D::engine::Vector2;
//sdl2 keys
use sdl2::keyboard::Keycode;
use sdl2::mouse::MouseButton;

//mods
mod object_example;

fn main() {
    let global_config = GlobalConfig::new(Resolution { width: 1024, height: 576}, String::from("Axle Tests"), 4_f32, 60);
    let rc_global_config = Rc::new(global_config);

    let mut app = App::new(rc_global_config);

    let mut new_object = object_example::Rectangle {
        position: Vector2::new(100_f32, 100_f32),
        size: Vector2::new(100_f32, 100_f32),
    };

    app.add_object(Box::new(new_object));
        
    loop {
        let (exit, frame_start) = app.update();

        if exit { break }

        if app.input.is_key_pressed(Keycode::W) {
            println!("Mouse left pressed");
        }

        if app.input.is_key_released(Keycode::W) {
            println!("Mouse left released");
        }

        if app.input.is_key_held(Keycode::W) {
            println!("Mouse left holding");
        }

        app.wait(frame_start);
    }
}

