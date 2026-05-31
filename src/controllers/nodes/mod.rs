use askama::Template;
use axum::{
    extract::{Path, State},
    http::StatusCode,
    response::{Html, Result},
};

use super::super::NAME as ISSUER;
use super::State as AppState;

pub async fn index(
    State(state): State<AppState>,
    Path(token): Path<String>,
) -> Result<Html<String>, (StatusCode, String)> {
    let body = Index {}
        .render()
        .map_err(|e| (StatusCode::INTERNAL_SERVER_ERROR, e.to_string()))?;
    let _ = state
        .jwt
        .verify(&token, ISSUER)
        .map_err(|x| (StatusCode::FORBIDDEN, x.to_string()))?;
    Ok(Html(body))
}

#[derive(Template)]
#[template(path = "nodes/index.html")]
struct Index {}
