use axle_engine::application::App;
use axle_engine::axle_math::vector::Vector2;
use axle_engine::config::GlobalConfig;

fn main() {
    let global_config = GlobalConfig::new(
        axle_engine::config::Resolution {
            width: 800,
            height: 600,
        },
        String::from("Example"),
        9.81,
        60,
    );

    let mut app = App::new();

    loop {
        app.update();
    }
}
