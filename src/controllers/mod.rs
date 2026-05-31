pub mod nodes;
pub mod pods;

use opensearch::OpenSearch as OpenSearchClient;
use std::sync::Arc;

use super::jwt::Jwt;

#[derive(Clone)]
pub struct State {
    pub jwt: Arc<Jwt>,
    pub search: Arc<OpenSearch>,
}

pub struct OpenSearch {
    pub client: OpenSearchClient,
    pub namespace: Option<String>,
}
