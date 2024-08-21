use std::{any::Any, cell::RefCell, rc::Rc};

use axle_ecs::World;
use eyre::Result;

#[test]
fn create_entity() -> Result<()> {
    let mut world = World::new();
    world.register_component::<Location>();
    world.register_component::<Size>();

    world
        .create_entity()
        .with_component(Location(42.0, 24.0))?
        .with_component(Size(10.0))?;

    Ok(())
}

#[test]
fn query_for_entities() -> Result<()> {
    let mut world = World::new();
    world.register_component::<Location>();
    world.register_component::<Size>();

    world
        .create_entity()
        .with_component(Location(42.0, 24.0))?
        .with_component(Size(10.0))?;

    world
        .create_entity()
        .with_component(Size(11.0))?;

    world
        .create_entity()
        .with_component(Location(43.0, 25.0))?;

    world
        .create_entity()
        .with_component(Location(44.0, 26.0))?
        .with_component(Size(12.0))?;

    let query = world.query()
        .with_component::<Location>()?
        .with_component::<Size>()?
        .run();

    let locations: &Vec<Rc<RefCell<dyn Any>>> = &query[0];
    let sizes: &Vec<Rc<RefCell<dyn Any>>> = &query[1];

    assert_eq!(locations.len(), sizes.len());
    assert_eq!(locations.len(), 2);

    let borrowed_first_location = locations[0].borrow();
    let first_location = borrowed_first_location.downcast_ref::<Location>().unwrap();
    let borrowed_first_size = sizes[0].borrow();
    let first_size = borrowed_first_size.downcast_ref::<Size>().unwrap();

    assert_eq!(first_size.0, 10.0);
    assert_eq!(first_location.0, 42.0);

    let borrowed_second_location = locations[1].borrow();
    let second_location = borrowed_second_location.downcast_ref::<Location>().unwrap();
    let borrowed_second_size = sizes[1].borrow();
    let second_size = borrowed_second_size.downcast_ref::<Size>().unwrap();

    assert_eq!(second_size.0, 12.0);
    assert_eq!(second_location.0, 44.0);

    Ok(())
}

struct Location(pub f32, pub f32);
struct Size(pub f32);