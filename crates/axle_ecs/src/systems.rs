use std::any::{Any, TypeId};
use eyre::Result;

use crate::entities::{query::Query, query_entity::QueryEntity, Entities};
use crate::CustomErrors;

pub type SystemFunction = &'static dyn Fn(&Vec<QueryEntity>) -> Result<()>;
pub type SystemComponents = Vec<TypeId>;

#[derive(Default)]
pub struct Systems {
    funtions: Vec<SystemFunction>,
    components: Vec<SystemComponents>,
    bit_masks: Vec<u32>,
    inserting_into_index: usize
}

impl Systems {
    pub fn create_system(&mut self, system: &'static dyn Fn(&Vec<QueryEntity>) -> Result<()>) -> &mut Self {
        self.inserting_into_index = self.funtions.len();
        self.bit_masks.push(1 << self.bit_masks.len());
        self.funtions.push(system);
        self.components.push(vec![]);

        self
    }

    pub fn with_component<T: Any>(&mut self) -> Result<&mut Self> {
        let type_id = TypeId::of::<T>();

        let components = self.components
            .get_mut(self.inserting_into_index)
            .ok_or(CustomErrors::CreateSystemNeverCalled)?;
        components.push(type_id);

        Ok(self)
    }

    pub fn get_bitmask(&self, index: usize) -> Option<u32> {
        self.bit_masks.get(index).copied()
    }

    pub fn run_all(&self, entities: &Entities) -> Result<()> {
        Ok(())
    }
}

#[cfg(test)]
mod tests {
    use std::any::Any;

    use crate::entities::{query_entity::QueryEntity, Entities};
    use super::*;

    #[test]
    fn add_system_to_entity() -> Result<()> {
        let mut entities = Entities::default();

        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(100))?
            .with_system(systems.get_bitmask(0).unwrap())?;

        assert_eq!(systems.bit_masks.len(), 1);
        assert_eq!(systems.bit_masks[0], 1);
        assert_eq!(systems.components[0].len(), 1);
        assert_eq!(systems.components[0][0], TypeId::of::<Health>());

        Ok(())
    }

    fn excecute_system() -> Result<()> {
        let mut entities = Entities::default();

        entities.register_component::<Health>();
        entities.register_component::<Speed>();

        let mut systems = Systems::default();
        systems
            .create_system(&damage_health)
            .with_component::<Health>()?;

        entities
            .create_entity()
            .with_component(Health(100))?
            .with_component(Speed(100))?
            .with_system(systems.get_bitmask(0).unwrap())?;

        //systems.run_all(&entities)?;

        let mut query = Query::new(&entities);
        let entities_query = query
            .with_component::<Health>()?
            .run_entity();

        assert_eq!(entities_query[0].get_component::<Health>()?.0, 90_u32);

        Ok(())
    }

    fn damage_health(entities: &Vec<QueryEntity>) -> Result<()> {
        for entity in entities {
            let mut health_mut = entity.get_component_mut::<Health>()?;
            health_mut.0 -= 10;
        }

        Ok(())
    }

    struct Health(pub u32);
    struct Speed(pub u32);
}
