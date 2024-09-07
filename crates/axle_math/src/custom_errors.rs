use thiserror::Error;

/// Custom errors that can occur when using the ECS.
#[derive(Debug, Error)]
pub enum CustomErrors {
    #[error("Could not normalize because these components: `{0}` because they are zero (or very close to) or either one of them is NaN or infinite")]
    CouldNotNormalize(&'static str),
}