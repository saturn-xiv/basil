pub mod app;
pub mod controllers;
pub mod jwt;
pub mod models;

use std::error::Error as StdError;
use std::fmt;
use std::result::Result as StdResult;

use hyper::StatusCode;

pub const NAME: &str = env!("CARGO_PKG_NAME");
pub type Error = Box<dyn StdError + Send + Sync>;
pub type Result<T> = StdResult<T, Error>;

#[derive(Debug)]
pub struct HttpError(pub StatusCode, pub Option<String>);
impl StdError for HttpError {}

impl fmt::Display for HttpError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.1 {
            Some(ref v) => v.fmt(f),
            None => self.0.fmt(f),
        }
    }
}

impl From<Error> for HttpError {
    fn from(err: Error) -> Self {
        Self(StatusCode::INTERNAL_SERVER_ERROR, Some(err.to_string()))
    }
}

pub type HttpResult<T> = StdResult<T, HttpError>;
