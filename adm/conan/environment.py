from __future__ import annotations

from pathlib import Path
from typing import Dict


def _join_path(entries: list[str], platform_name: str) -> str:
    sep = ";" if platform_name.lower().startswith("win") else ":"
    return sep.join(entries)


def render_env_files(build_dir: Path, thirdparty_dir: Path, platform_name: str) -> Dict[str, str]:
    entries = [
        str(build_dir / "bin"),
        str(thirdparty_dir / "bin"),
    ]
    tail = "%PATH%" if platform_name.lower().startswith("win") else "$PATH"
    entries.append(tail)
    path_value = _join_path(entries, platform_name)

    env_files: Dict[str, str] = {}
    env_files["environment.ps1.env"] = f"PATH={path_value}"
    env_files["environment.bat.env"] = f"PATH={path_value}"
    env_files["environment.sh.env"] = f"PATH={path_value}"
    return env_files


def render_activate_files(platform_name: str) -> Dict[str, str]:
    return {
        "activate.bat": "FOR /F \"tokens=*\" %%i in (environment.bat.env) do SET %%i",
        "activate.ps1": "Get-Content environment.ps1.env | foreach { $k, $v = $_.split('='); Set-Content env:\\$k $v }",
        "activate.sh": "set -a; source \"environment.sh.env\"; set +a",
    }
