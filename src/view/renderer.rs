use std::rc::Rc;

use crate::config::GlobalConfig;
use crate::ecs::game_object::GameObject;
use crate::physics::shape::Shape;

//sdl2
use sdl2::Sdl;
use sdl2::render::Canvas;
use sdl2::video::Window;
use sdl2::pixels::Color;
use sdl2::VideoSubsystem;
use sdl2::gfx::primitives::DrawRenderer;
use sdl2::rect::Rect;

/// Structure that handles the rendering of the game objects.
///
/// This structure initializes the SDL context and video subsystem with the
/// specified title and resolution. It also creates a canvas to draw the game
/// objects on.
pub struct Renderer {
    // Main parameters
    /// The SDL context.
    //main parameters
    pub sdl_context: Sdl,
    /// The video subsystem.
    pub video_subsystem: VideoSubsystem,
    /// The canvas to draw the game objects on.
    pub canvas: Canvas<Window>,

    // Other not as important parameters
    /// The background color of the canvas.
    //other not as important parameters
    pub background_color: Color,
    /// The global configuration.
    pub config: Rc<GlobalConfig>
}

impl Renderer {
    /// Creates a new renderer with the specified configuration.
    ///
    /// # Arguments
    ///
    /// * `config` - The global configuration.
    ///
    /// # Returns
    ///
    /// The newly created renderer.
    pub fn new(config: Rc<GlobalConfig>) -> Self {
        // SDL initialization
        //sdl2 initialization
        let sdl_context = sdl2::init().unwrap();
        let video_subsystem = sdl_context.video().unwrap();
        let window = video_subsystem.window(&config.title, config.resolution.width, config.resolution.height)
            .position_centered()
            .build()
            .unwrap();
        let canvas = window.into_canvas().build().unwrap();

        Self {
            sdl_context,
            video_subsystem,
            //window,
            canvas,
            background_color: Color::GRAY,
            config
        }
    }

    /// Renders the game objects on the canvas.
    ///
    /// # Arguments
    ///
    /// * `objects` - The vector of game objects to render.
    pub fn render(&mut self, objects: &mut Vec<Box<dyn GameObject>>) {
        // Background
        //background
        self.canvas.set_draw_color(self.background_color);
        self.canvas.clear();

        // Game objects
        for object in objects {
            // Render objects
            let shape = object.get_shape();

            
            match shape {
                Shape::Circle { radius } => {
                    // Render circle
                    self.canvas.filled_circle(object.get_position().x as i16, object.get_position().y as i16, radius.clone() as i16, Color::RED).unwrap_or_default();
                },
                Shape::Rectangle { width, height } => {
                    // Render rectangle
                    self.canvas.fill_rect(Rect::new(object.get_position().x as i32, object.get_position().y as i32, width.clone() as u32, height.clone() as u32)).unwrap_or_default();
                }
            }
        }

        // Present the render
        self.canvas.present();
    }
}

