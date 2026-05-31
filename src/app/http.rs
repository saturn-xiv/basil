use std::path::Path;

use serde::{Deserialize, Serialize};

use super::super::Result;
use super::parse_toml;

pub async fn launch<P: AsRef<Path>>(config: P, port: u16) -> Result<()> {
    let config: Config = parse_toml(config)?;
    Ok(())
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct Config {
    #[serde(rename = "jwt-key")]
    jwt_key: String,
    opensearch: OpenSearch,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct OpenSearch {
    endpoint: String,
    namespace: Option<String>,
}
