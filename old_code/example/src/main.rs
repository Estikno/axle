use axle_engine::application::App;

fn main() {
    let mut app = App::new();

    app.start().unwrap();

    loop {
        app.update().unwrap();
    }
}
