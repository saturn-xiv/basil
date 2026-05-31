use std::process::ExitCode;

#[tokio::main]
async fn main() -> ExitCode {
    env_logger::init();
    if let Err(e) = basil::app::run().await {
        log::error!("{}", e);
        return ExitCode::FAILURE;
    }
    ExitCode::SUCCESS
}
