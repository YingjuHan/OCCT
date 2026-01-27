from __future__ import annotations

import json
from pathlib import Path
from typing import Dict


def _preset_name(kit: str, build_type: str) -> str:
    return f"{kit}-{build_type.strip().lower()}"


def render_workspace(source_dir: Path, build_root: Path, kit: str, build_type: str) -> Dict[str, object]:
    preset = _preset_name(kit, build_type)
    return {
        "folders": [{"path": str(source_dir)}],
        "settings": {
            "cmake.useCMakePresets": "always",
            "cmake.sourceDirectory": "${workspaceFolder}",
            "cmake.configureOnOpen": True,
            "cmake.configurePreset": preset,
            "cmake.buildPreset": preset,
            "cmake.testPreset": preset,
        },
    }


def write_workspace(path: Path, data: Dict[str, object]) -> None:
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")
