use std::env;
use std::fs::File;
use std::io::Write;
use std::path::Path;
use std::process::Command;

fn main() -> Result<(), Box<dyn std::error::Error>> {
    {
        let out_dir = env::var("OUT_DIR")?;
        let git_version = shell(
            Command::new("git")
                .arg("describe")
                .arg("--tags")
                .arg("--always")
                .arg("--first-parent")
                .arg("--dirty"),
        )?;
        let build_time = shell(Command::new("date").arg("-u").arg("-R"))?;

        let dest_path = Path::new(&out_dir).join("env.rs");
        let mut fd = File::create(dest_path)?;

        writeln!(fd, r#"pub const GIT_VERSION: &str = "{git_version}";"#)?;
        writeln!(fd, r#"pub const BUILD_TIME: &str = "{build_time}";"#)?;
    }

    Ok(())
}

fn shell(cmd: &mut Command) -> Result<String, Box<dyn std::error::Error>> {
    let it = cmd.output()?;
    let it = std::str::from_utf8(&it.stdout)?;
    let it = it.trim();
    Ok(it.to_string())
}
