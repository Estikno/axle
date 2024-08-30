use eyre::Result;

use crate::entities::{query::Query, Entities};

pub type System_function = Box<dyn Fn(&mut Query) -> Result<()>>;

#[derive(Default)]
pub struct Systems {
    data: Vec<Vec<System_function>>
}

impl Systems {
    pub fn add_system(&mut self, system: &dyn Fn(Query) -> Result<()>) -> Result<&mut Self> {
        Ok(self)
    }

    pub fn to_entity_by_id(&mut self, index: usize) -> Result<&mut Self> {
        Ok(self)
    }

    pub fn run_all(&mut self) {

    }
}

#[cfg(test)]
mod tests {
    use crate::entities::Entities;
    use super::*;

    #[test]
    fn add_system_to_entity() -> Result<()> {
        let mut entities = Entities::default();

        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(100))?;

        let mut systems = Systems::default();

        systems
            .add_system(&damage_health)?
            .to_entity_by_id(0)?;

        systems.run_all();

        let mut query = Query::new(&entities);
        let entities_query = query
            .with_component::<Health>()?
            .run_entity();

        assert_eq!(entities_query[0].get_component::<Health>()?.0, 90_u32);

        Ok(())
    }

    fn damage_health(mut query: Query) -> Result<()> {
        let entities = query
            .with_component::<Health>()?
            .run_entity();

        for entity in entities {
            let mut health_mut = entity.get_component_mut::<Health>()?;
            health_mut.0 -= 10;
        }

        Ok(())
    }

    struct Health(pub u32);
    struct Speed(pub u32);
}