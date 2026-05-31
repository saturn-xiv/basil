use std::net::SocketAddr;
use std::path::Path;
use std::sync::Arc;

use axum::{Router, routing::get};
use opensearch::{
    OpenSearch as OpenSearchClient,
    http::{
        Url,
        transport::{SingleNodeConnectionPool, TransportBuilder},
    },
};
use serde::{Deserialize, Serialize};
use tokio::signal;

use super::super::{Result, controllers, jwt::Jwt};
use super::parse_toml;

pub async fn launch<P: AsRef<Path>>(config: P, port: u16) -> Result<()> {
    let config: Config = parse_toml(config)?;
    let state = controllers::State {
        jwt: Arc::new(Jwt::new(&config.jwt_key)?),
        search: Arc::new(controllers::OpenSearch {
            client: config.opensearch.open()?,
            namespace: config.opensearch.namespace.clone(),
        }),
    };
    let app = Router::new()
        .route("/{token}/pods/{name}", get(controllers::pods::show::get))
        .route("/{token}/pods", get(controllers::pods::index))
        .route("/{token}/nodes", get(controllers::nodes::index))
        .with_state(state);
    let addr = SocketAddr::from(([0, 0, 0, 0], port));
    log::info!("listening on http://{addr}");
    let listener = tokio::net::TcpListener::bind(&addr).await.unwrap();
    axum::serve(listener, app)
        .with_graceful_shutdown(shutdown_signal())
        .await?;
    Ok(())
}

async fn shutdown_signal() {
    let ctrl_c = async {
        signal::ctrl_c()
            .await
            .expect("failed to install Ctrl+C handler");
    };

    let terminate = async {
        signal::unix::signal(signal::unix::SignalKind::terminate())
            .expect("failed to install signal handler")
            .recv()
            .await;
    };

    tokio::select! {
        _ = ctrl_c => {},
        _ = terminate => {},
    }
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

impl OpenSearch {
    fn open(&self) -> Result<OpenSearchClient> {
        let url = Url::parse(&self.endpoint)?;
        let transport = TransportBuilder::new(SingleNodeConnectionPool::new(url))
            .disable_proxy()
            .build()?;
        Ok(OpenSearchClient::new(transport))
    }
}
