use std::path::Path;

use serde::{Deserialize, Serialize};

use super::super::{NAME, Result, jwt::Jwt};
use super::parse_toml;

pub fn generate<P: AsRef<Path>>(config: P, user: &str, years: u8) -> Result<()> {
    let config: Config = parse_toml(config)?;
    let jwt = Jwt::new(&config.jwt_key)?;
    let token = jwt.by_years(NAME, user, years)?;
    println!("{token}");
    Ok(())
}

#[derive(Debug, Clone, Serialize, Deserialize)]
struct Config {
    #[serde(rename = "jwt-key")]
    jwt_key: String,
}
