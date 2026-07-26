from pathlib import Path

Import("env")


def load_env_file(path: Path) -> dict:
    values = {}
    if not path.is_file():
        return values

    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue

        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip().strip('"').strip("'")
        if key:
            values[key] = value

    return values


project_dir = Path(str(env.subst("$PROJECT_DIR")))
env_file = project_dir / "src" / ".env"
settings = load_env_file(env_file)

ssid = settings.get("AP_SSID", "")
password = settings.get("AP_PASS", "")

if not ssid or not password:
    raise ValueError("Missing AP_SSID or AP_PASS in src/.env")

generated_header = project_dir / "src" / "ap_config.h"
generated_header.write_text(
    "#ifndef AP_CONFIG_H\n"
    "#define AP_CONFIG_H\n\n"
    f'#define AP_SSID "{ssid}"\n'
    f'#define AP_PASS "{password}"\n\n'
    "#endif // AP_CONFIG_H\n",
    encoding="utf-8",
)