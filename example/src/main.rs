use std::rc::Rc;

use axle2D::app::App;
use axle2D::config::{GlobalConfig, Resolution};

fn main() {
    let global_config = GlobalConfig::new(Resolution { width: 1024, height: 576}, String::from("Axle Tests"), 4_f32, 60);
    let rc_global_config = Rc::new(global_config);

    let mut app = App::new(rc_global_config);
        
    app.update();
}

