from __future__ import annotations

import json
from pathlib import Path
from typing import Dict, Iterable, List


def _preset_name(kit: str, build_type: str) -> str:
    suffix = build_type.strip().lower()
    mapping = {
        "relwithdebinfo": "relwithdebinfo",
        "minsizerel": "minsizerel",
    }
    suffix = mapping.get(suffix, suffix)
    return f"{kit}-{suffix}"


def render_user_presets(
    source_dir: Path,
    build_root: Path,
    kit: str,
    build_types: Iterable[str],
    cache_vars: Dict[str, object],
    toolchain_file: Path,
) -> Dict[str, object]:
    configure_presets: List[Dict[str, object]] = []
    build_presets: List[Dict[str, object]] = []
    test_presets: List[Dict[str, object]] = []

    for build_type in build_types:
        preset_name = _preset_name(kit, build_type)
        binary_dir = build_root / kit / build_type
        configure_presets.append(
            {
                "name": preset_name,
                "displayName": f"{kit} {build_type}",
                "generator": None,
                "binaryDir": str(binary_dir),
                "cacheVariables": dict(cache_vars),
                "toolchainFile": str(toolchain_file),
            }
        )
        build_presets.append({"name": preset_name, "configurePreset": preset_name})
        test_presets.append({"name": preset_name, "configurePreset": preset_name})

    return {
        "version": 4,
        "cmakeMinimumRequired": {"major": 3, "minor": 23, "patch": 0},
        "configurePresets": configure_presets,
        "buildPresets": build_presets,
        "testPresets": test_presets,
    }


def write_user_presets(path: Path, data: Dict[str, object]) -> None:
    path.write_text(json.dumps(data, indent=2), encoding="utf-8")
