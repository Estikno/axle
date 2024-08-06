use std::collections::HashMap;

//sdl2
use sdl2::event::Event;
use sdl2::keyboard::Keycode;
use sdl2::mouse::MouseButton;
use sdl2::EventPump;

#[derive(Copy, Clone, PartialEq)]
pub enum ButtonState {
    Pressed,
    Released,
    Held,
    None
}

pub struct Input {
    mouse_button_state: HashMap<MouseButton, ButtonState>,
    key_state: HashMap<Keycode, ButtonState>,
    event_pump: EventPump
}

impl Input {
    pub fn new(sdl_context: &sdl2::Sdl) -> Self {
        Input {
            mouse_button_state: HashMap::new(),
            key_state: HashMap::new(),
            event_pump: sdl_context.event_pump().unwrap()
        }
    }

    pub fn update(&mut self) {
        // Reset states from the previous frame
        for state in self.mouse_button_state.values_mut() {
            match *state {
                ButtonState::Pressed => *state = ButtonState::Held,
                ButtonState::Released => *state = ButtonState::None,
                _ => {},
            }
        }

        for state in self.key_state.values_mut() {
            match *state {
                ButtonState::Pressed => *state = ButtonState::Held,
                ButtonState::Released => *state = ButtonState::None,
                _ => {},
            }
        }
    }

    pub fn process_event(&mut self) -> bool {
        for event in self.event_pump.poll_iter() {
            match event {
                Event::Quit { .. } | 
                Event::KeyDown { keycode: Some(Keycode::Escape), .. } => {
                    return true;
                }
                Event::MouseButtonDown { mouse_btn, .. } => {
                    self.mouse_button_state.insert(mouse_btn, ButtonState::Pressed);
                }
                Event::MouseButtonUp { mouse_btn, .. } => {
                    self.mouse_button_state.insert(mouse_btn, ButtonState::Released);
                }
                Event::KeyDown { keycode, repeat, .. } => {
                    if !repeat {
                        self.key_state.insert(keycode.unwrap(), ButtonState::Pressed);
                    }
                }
                Event::KeyUp { keycode, .. } => {
                    self.key_state.insert(keycode.unwrap(), ButtonState::Released);
                }
                _ => {}
            }
        }

        false
    }

    pub fn is_mouse_button_pressed(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Pressed)
    }

    pub fn is_mouse_button_held(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Held)
    }

    pub fn is_mouse_button_released(&self, button: MouseButton) -> bool {
        self.mouse_button_state
            .get(&button)
            .map_or(false, |&state| state == ButtonState::Released)
    }

    pub fn is_key_pressed(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Pressed)
    }

    pub fn is_key_held(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Held)
    }

    pub fn is_key_released(&self, keycode: Keycode) -> bool {
        self.key_state
            .get(&keycode)
            .map_or(false, |&state| state == ButtonState::Released)
    }
}