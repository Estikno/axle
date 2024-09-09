use thiserror::Error;

/// Custom errors that can occur when using the ECS.
#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error(
        "Attempting to access a render config that has not been registered in the ecs's resources."
    )]
    RenderConfigNotRegistered,
}
