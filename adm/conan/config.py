from __future__ import annotations

import re
from pathlib import Path
from typing import Tuple

_VERSION_PATTERN = re.compile(r"set\s*\(OCC_VERSION_(MAJOR|MINOR|MAINTENANCE|DEVELOPMENT)\s+([^\s\)]+)")


def parse_version_cmake(text: str) -> Tuple[int, int, int, str | None]:
    values: dict[str, str] = {}
    for line in text.splitlines():
        match = _VERSION_PATTERN.search(line)
        if not match:
            continue
        key = match.group(1)
        raw = match.group(2).strip().strip('"')
        values[key] = raw
    major = int(values.get("MAJOR", "0"))
    minor = int(values.get("MINOR", "0"))
    maintenance = int(values.get("MAINTENANCE", "0"))
    dev = values.get("DEVELOPMENT")
    return major, minor, maintenance, dev


def version_string(version: Tuple[int, int, int, str | None]) -> str:
    major, minor, maintenance, dev = version
    base = f"{major}.{minor}.{maintenance}"
    return f"{base}-{dev}" if dev else base


def repo_root(start: Path | None = None) -> Path:
    here = Path(start or __file__).resolve()
    for parent in [here] + list(here.parents):
        if (parent / "CMakeLists.txt").is_file():
            return parent
    return here.parents[2]


def read_version(root: Path | None = None) -> str:
    root = root or repo_root()
    version_file = root / "adm" / "cmake" / "version.cmake"
    content = version_file.read_text(encoding="utf-8")
    return version_string(parse_version_cmake(content))
