use std::rc::Rc;

use crate::config::GlobalConfig;
use crate::ecs::game_object::GameObject;
use crate::engine::Vector2;
use crate::physics::shape::Shape;
use crate::utils::coordinate::convert_vector_y;

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
        self.canvas.set_draw_color(self.background_color);
        self.canvas.clear();

        // Game objects
        for object in objects {
            // Render objects
            let position_adjusted = convert_vector_y(&object.get_position());
            let transform = object.transform().clone();
            
            if object.transform_mut().has_changed() {
                let shape = object.get_shape_mut();
                shape.need_to_update_transformed_vertices();
                shape.update_transform_vertices(&transform);
            }
            
            let shape = object.get_shape();

            match shape {
                Shape::Circle { radius } => {
                    // Render circle
                    self.canvas.filled_circle(position_adjusted.x as i16, position_adjusted.y as i16, radius.clone() as i16, Color::RED).unwrap_or_default();
                },
                Shape::Rectangle { width, height, tranformed_vertices, triangles, color, .. } => {
                    // Render rectangle as two triangles to manage rotations
                    /*self.canvas.filled_trigon(
                        convert_vector_y(&tranformed_vertices[triangles[0] as usize]).x as i16, 
                        convert_vector_y(&tranformed_vertices[triangles[0] as usize]).y as i16, 
                        convert_vector_y(&tranformed_vertices[triangles[1] as usize]).x as i16, 
                        convert_vector_y(&tranformed_vertices[triangles[1] as usize]).y as i16, 
                        convert_vector_y(&tranformed_vertices[triangles[2] as usize]).x as i16,
                        convert_vector_y(&tranformed_vertices[triangles[2] as usize]).y as i16,
                        Color::GREEN
                    ).unwrap_or_default();
                    
                    self.canvas.filled_trigon(
                        convert_vector_y(&tranformed_vertices[triangles[3] as usize]).x as i16,
                        convert_vector_y(&tranformed_vertices[triangles[3] as usize]).y as i16,
                        convert_vector_y(&tranformed_vertices[triangles[4] as usize]).x as i16,
                        convert_vector_y(&tranformed_vertices[triangles[4] as usize]).y as i16, 
                        convert_vector_y(&tranformed_vertices[triangles[5] as usize]).x as i16,
                        convert_vector_y(&tranformed_vertices[triangles[5] as usize]).y as i16, 
                        Color::GREEN
                    ).unwrap_or_default();*/

                    self.draw_triangle(
                        &convert_vector_y(&tranformed_vertices[triangles[0] as usize]), 
                        &convert_vector_y(&tranformed_vertices[triangles[1] as usize]), 
                        &convert_vector_y(&tranformed_vertices[triangles[2] as usize]), 
                        color.clone()
                    );

                    self.draw_triangle(
                        &convert_vector_y(&tranformed_vertices[triangles[3] as usize]), 
                        &convert_vector_y(&tranformed_vertices[triangles[4] as usize]), 
                        &convert_vector_y(&tranformed_vertices[triangles[5] as usize]), 
                        color.clone()
                    );
                }
            }
        }

        // Present the render
        self.canvas.present();
    }

    fn draw_triangle(&mut self, pos_1: &Vector2, pos_2: &Vector2, pos_3: &Vector2, color: Color) {
        self.canvas.filled_trigon(
            pos_1.x as i16, 
            pos_1.y as i16, 
            pos_2.x as i16, 
            pos_2.y as i16, 
            pos_3.x as i16,
            pos_3.y as i16,
            color
        ).unwrap_or_default()
    }
}

