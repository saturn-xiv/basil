#[tokio::main]
async fn main() {
    env_logger::init();
    if let Err(e) = basil::app::run().await {
        log::error!("{}", e);
    }
}
