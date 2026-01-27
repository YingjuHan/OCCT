from __future__ import annotations

import os
import subprocess
import sys
from pathlib import Path
from typing import List


def venv_python(venv_dir: Path, platform_name: str | None = None) -> Path:
    platform_name = platform_name or os.name
    if platform_name.lower().startswith("win") or platform_name == "nt":
        return venv_dir / "Scripts" / "python.exe"
    return venv_dir / "bin" / "python"


def build_reexec_args(python_exe: Path, argv: List[str]) -> List[str]:
    return [python_exe.as_posix()] + list(argv)


def should_reexec(target_exe: Path, current_exe: Path) -> bool:
    return target_exe.resolve() != current_exe.resolve()


def reexec_if_needed(target_exe: Path) -> None:
    if os.environ.get("OCCT_CONAN_BOOTSTRAPPED") == "1":
        return
    if not should_reexec(target_exe, Path(sys.executable)):
        return
    env = os.environ.copy()
    env["OCCT_CONAN_BOOTSTRAPPED"] = "1"
    os.execve(str(target_exe), build_reexec_args(target_exe, sys.argv), env)


def ensure_conan_venv(repo_root: Path) -> Path:
    venv_dir = repo_root / ".venv-conan"
    python_exe = venv_python(venv_dir)
    if not python_exe.exists():
        subprocess.check_call([sys.executable, "-m", "venv", str(venv_dir)])
        subprocess.check_call([str(python_exe), "-m", "pip", "install", "--upgrade", "pip"])
        subprocess.check_call([str(python_exe), "-m", "pip", "install", "conan"])
    return python_exe
