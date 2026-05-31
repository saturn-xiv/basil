pub mod nodes;
pub mod pods;

use std::sync::Arc;

use super::jwt::Jwt;

#[derive(Clone)]
pub struct State {
    pub jwt: Arc<Jwt>,
}
