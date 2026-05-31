pub mod http;
pub mod token;

use std::fs;
use std::os::unix::fs::PermissionsExt;
use std::path::{Path, PathBuf};

use clap::{Parser, Subcommand};
use hyper::StatusCode;
use serde::de::DeserializeOwned;

use super::{HttpError, Result};

const NAME: &str = env!("CARGO_PKG_NAME");
const DESCRIPTION: &str = env!("CARGO_PKG_DESCRIPTION");
const HOMEPAGE: &str = env!("CARGO_PKG_HOMEPAGE");
const BANNER: &str = include_str!("banner.txt");

include!(concat!(env!("OUT_DIR"), "/env.rs"));

pub async fn run() -> Result<()> {
    let args = Cli::parse();
    match args.command {
        Commands::GenerateToken { ref user, years } => token::generate(&args.config, user, years),
        Commands::Http { port } => http::launch(&args.config, port).await,
    }
}

#[derive(Debug, Parser)]
#[command(name = NAME, version = GIT_VERSION, about = DESCRIPTION, before_help = BANNER, after_help = HOMEPAGE, long_about = None, propagate_version = true, arg_required_else_help = true)]
struct Cli {
    #[arg(
        short,
        long,
        help = "Load configuration from file(toml)",
        default_value = "config.toml"
    )]
    config: PathBuf,
    #[command(subcommand)]
    command: Commands,
}

#[derive(Debug, Subcommand)]
enum Commands {
    #[command(about = "Generate a new token")]
    GenerateToken {
        #[arg(short, long, required = true, help = "Username")]
        user: String,
        #[arg(short, long, required = true, help = "Years")]
        years: u8,
    },
    #[command(about = "Launch a HTTP server")]
    Http {
        #[arg(short, long, required = true, help = "Port")]
        port: u16,
    },
}

fn check_permission<P: AsRef<Path>>(file: P) -> Result<()> {
    match fs::metadata(file)?.permissions().mode() & 0o777 {
        0o400 | 0o600 => Ok(()),
        v => Err(Box::new(HttpError(
            StatusCode::FORBIDDEN,
            Some(format!("file permission is too open({:#03o})", v)),
        ))),
    }
}
fn parse_toml<P: AsRef<Path>, T: DeserializeOwned>(file: P) -> Result<T> {
    let cfg = file.as_ref();
    log::debug!("load configuration from {}", cfg.display());
    check_permission(cfg)?;
    let it: T = toml::from_str(&fs::read_to_string(cfg)?)?;
    Ok(it)
}
