use std::collections::HashMap;

//sdl2
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use sdl2::mouse::MouseButton;
use sdl2::EventPump;

/// Enum to represent the state of a button.
#[derive(Copy, Clone, PartialEq)]
pub enum ButtonState {
    /// The button is currently being pressed.
    Pressed,
    /// The button was released in the previous frame.
    Released,
    /// The button is currently being held.
    Held,
    /// The button is not being pressed or held.
    None,
}

/// Structure to handle input from the user.
pub struct Input {
    /// The state of all the mouse buttons.
    mouse_button_state: HashMap<MouseButton, ButtonState>,
    /// The state of all the keyboard keys.
    key_state: HashMap<Keycode, ButtonState>,
    /// The SDL event pump to get events from.
    event_pump: EventPump,
}

impl Input {
    /// Create a new `Input` instance with the given SDL context.
    pub fn new(sdl_context: &sdl2::Sdl) -> Self {
        Input {
            mouse_button_state: HashMap::new(),
            key_state: HashMap::new(),
            event_pump: sdl_context.event_pump().unwrap(),
        }
    }

    /// Resets the state of all buttons to their state in the previous frame.
    pub fn update(&mut self) {
        // Reset states from the previous frame
        for state in self.mouse_button_state.values_mut() {
            match *state {
                ButtonState::Pressed => *state = ButtonState::Held,
                ButtonState::Released => *state = ButtonState::None,
                _ => {}
            }
        }

        for state in self.key_state.values_mut() {
            match *state {
                ButtonState::Pressed => *state = ButtonState::Held,
                ButtonState::Released => *state = ButtonState::None,
                _ => {}
            }
        }
    }

    /// Processes a single frame of events from the SDL event pump.
    ///
    /// Returns `true` if the user has quit the game, and `false` otherwise.
    pub fn process_event(&mut self) -> bool {
        for event in self.event_pump.poll_iter() {
            match event {
                Event::Quit { .. }
                | Event::KeyDown {
                    keycode: Some(Keycode::Escape),
                    ..
                } => {
                    return true;
                }
                Event::MouseButtonDown { mouse_btn, .. } => {
                    self.mouse_button_state
                        .insert(mouse_btn, ButtonState::Pressed);
                }
                Event::MouseButtonUp { mouse_btn, .. } => {
                    self.mouse_button_state
                        .insert(mouse_btn, ButtonState::Released);
                }
                Event::KeyDown {
                    keycode, repeat, ..
                } => {
                    if !repeat {
                        self.key_state
                            .insert(keycode.unwrap(), ButtonState::Pressed);
                    }
                }
                Event::KeyUp { keycode, .. } => {
                    self.key_state
                        .insert(keycode.unwrap(), ButtonState::Released);
                }
                _ => {}
            }
        }

        false
    }

    /// Checks if a mouse button is currently being pressed.
    pub fn is_mouse_button_pressed(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Pressed)
    }

    /// Checks if a mouse button is currently being held.
    pub fn is_mouse_button_held(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Held)
    }

    /// Checks if a mouse button was released in the previous frame.
    pub fn is_mouse_button_released(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Released)
    }

    /// Checks if a key is currently being pressed.
    pub fn is_key_pressed(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Pressed)
    }

    /// Checks if a key is currently being held.
    pub fn is_key_held(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Held)
    }

    /// Checks if a key was released in the previous frame.
    pub fn is_key_released(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Released)
    }
}
