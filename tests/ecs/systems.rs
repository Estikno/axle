use eyre::Result;

use axle_ecs::{entities::query_entity::QueryEntity, resources::Resources, World};

#[test]
fn create_system() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_system(&update_location)
        .with_component::<Location>()?
        .with_component::<Speed>()?;

    Ok(())
}

#[test]
fn running_system() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_entity()
        .with_component(Location(0.0, 10.0))?
        .with_component(Speed(10.0))?;

    world
        .create_system(&update_location)
        .with_component::<Location>()?
        .with_component::<Speed>()?;

    world.run_all_systems()?;

    let query = world.query().with_component::<Location>()?.run();

    let locations = &query.1[0];
    let wrapped_location = locations[0].borrow();
    let location = wrapped_location.downcast_ref::<Location>().unwrap();

    assert_eq!(location.0, 10.0);
    assert_eq!(location.1, 20.0);

    Ok(())
}

#[test]
fn delete_component_from_system() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_entity()
        .with_component(Location(0.0, 10.0))?
        .with_component(Speed(10.0))?;

    world.create_entity().with_component(Speed(20.0))?;

    world
        .create_system(&update_speed)
        .with_component::<Location>()?
        .with_component::<Speed>()?;

    world.delete_component_by_system_id::<Location>(0)?;

    world.run_all_systems()?;

    let query = world.query().with_component::<Speed>()?.run();

    let speeds = &query.1[0];

    let wrapped_first_speed = speeds[0].borrow();
    let first_speed = wrapped_first_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(first_speed.0, 20.0);

    let wrapped_second_speed = speeds[1].borrow();
    let second_speed = wrapped_second_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(second_speed.0, 30.0);

    Ok(())
}

#[test]
fn add_component_to_system() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_entity()
        .with_component(Location(0.0, 10.0))?
        .with_component(Speed(10.0))?;

    world.create_entity().with_component(Location(10.0, 10.0))?;

    world.create_system(&update_location);

    world.add_component_to_system_by_id::<Location>(0)?;
    world.add_component_to_system_by_id::<Speed>(0)?;

    world.run_all_systems()?;

    let query = world.query().with_component::<Location>()?.run();

    let locations = &query.1[0];

    let wrapped_first_location = locations[0].borrow();
    let first_location = wrapped_first_location.downcast_ref::<Location>().unwrap();
    assert_eq!(first_location.0, 10.0);
    assert_eq!(first_location.1, 20.0);

    let wrapped_second_location = locations[1].borrow();
    let second_location = wrapped_second_location.downcast_ref::<Location>().unwrap();
    assert_eq!(second_location.0, 10.0);
    assert_eq!(second_location.1, 10.0);

    Ok(())
}

#[test]
fn delete_a_system() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_entity()
        .with_component(Location(0.0, 10.0))?
        .with_component(Speed(10.0))?;

    world.create_entity().with_component(Speed(20.0))?;

    world
        .create_system(&update_speed)
        .with_component::<Speed>()?;

    world.delete_system_by_id(0)?;
    world.run_all_systems()?;

    let query = world.query().with_component::<Speed>()?.run();

    let speeds = &query.1[0];

    let wrapped_first_speed = speeds[0].borrow();
    let first_speed = wrapped_first_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(first_speed.0, 10.0);

    let wrapped_second_speed = speeds[1].borrow();
    let second_speed = wrapped_second_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(second_speed.0, 20.0);

    Ok(())
}

#[test]
fn more_systems_at_the_same_time() -> Result<()> {
    let mut world = World::new();

    world.register_component::<Location>();
    world.register_component::<Speed>();

    world
        .create_entity()
        .with_component(Location(0.0, 0.0))?
        .with_component(Speed(10.0))?;

    world.create_entity().with_component(Speed(20.0))?;

    world.create_entity().with_component(Location(20.0, 20.0))?;

    world
        .create_entity()
        .with_component(Location(10.0, 10.0))?
        .with_component(Speed(5.0))?;

    world
        .create_system(&update_speed)
        .with_component::<Speed>()?;

    world
        .create_system(&update_location)
        .with_component::<Location>()?
        .with_component::<Speed>()?;

    world.run_all_systems()?;

    let query = world.query().with_component::<Speed>()?.run();

    let speeds = &query.1[0];

    let wrapped_first_speed = speeds[0].borrow();
    let first_speed = wrapped_first_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(first_speed.0, 20.0);

    let wrapped_second_speed = speeds[1].borrow();
    let second_speed = wrapped_second_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(second_speed.0, 30.0);

    let wrapped_third_speed = speeds[2].borrow();
    let third_speed = wrapped_third_speed.downcast_ref::<Speed>().unwrap();
    assert_eq!(third_speed.0, 15.0);

    let query = world.query().with_component::<Location>()?.run();

    let locations = &query.1[0];

    let wrapped_first_location = locations[0].borrow();
    let first_location = wrapped_first_location.downcast_ref::<Location>().unwrap();
    assert_eq!(first_location.0, 20.0);
    assert_eq!(first_location.1, 20.0);

    let wrapped_second_location = locations[1].borrow();
    let second_location = wrapped_second_location.downcast_ref::<Location>().unwrap();
    assert_eq!(second_location.0, 20.0);
    assert_eq!(second_location.1, 20.0);

    let wrapped_third_location = locations[2].borrow();
    let third_location = wrapped_third_location.downcast_ref::<Location>().unwrap();
    assert_eq!(third_location.0, 25.0);
    assert_eq!(third_location.1, 25.0);

    Ok(())
}

fn update_location(entities: &Vec<QueryEntity>, resources: &mut Resources) -> Result<()> {
    for entity in entities {
        let mut location = entity.get_component_mut::<Location>()?;
        let speed = entity.get_component::<Speed>()?;

        location.0 += speed.0;
        location.1 += speed.0;
    }

    Ok(())
}

fn update_speed(entities: &Vec<QueryEntity>, resources: &mut Resources) -> Result<()> {
    for entity in entities {
        let mut speed = entity.get_component_mut::<Speed>()?;
        speed.0 += 10.0;
    }

    Ok(())
}

struct Location(pub f32, pub f32);
struct Speed(pub f32);

