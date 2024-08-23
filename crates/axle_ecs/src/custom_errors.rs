use thiserror::Error;

#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error("Attempting to add component to an entity without calling create component first")]
    CreateComponentNeverCalled,
    #[error("Attempting to reference a component that wasn't registered")]
    ComponentNotRegistered,
    #[error("Attempting to reference an entity that doesn't exist")]
    EntityDoesNotExist,
}