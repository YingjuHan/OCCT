from __future__ import annotations

from typing import Dict, Iterable

BOOL_TRUE = {"1", "true", "yes", "on"}
BOOL_FALSE = {"0", "false", "no", "off"}

DEFAULT_OPTIONS: Dict[str, bool] = {
    "tcl": True,
    "tk": False,
    "freetype": True,
    "freeimage": False,
    "rapidjson": False,
    "tbb": False,
    "vtk": False,
    "draco": False,
    "ffmpeg": False,
    "openvr": False,
    "jemalloc": False,
    "gtest": False,
    "docs": False,
    "opengl": True,
    "angle": False,
    "d3d": False,
    "pch": False,
    "cpp20": False,
    "cpp23": False,
    "optimized": False,
    "shared": True,
    "clean": False,
}


def normalize_options(overrides: Dict[str, bool] | None) -> Dict[str, bool]:
    normalized = dict(DEFAULT_OPTIONS)
    if overrides:
        normalized.update({name: bool(value) for name, value in overrides.items()})
    if normalized.get("tk"):
        normalized["tcl"] = True
    if normalized.get("angle"):
        normalized["opengl"] = False
    return normalized


def parse_option_pairs(pairs: Iterable[str]) -> Dict[str, bool]:
    overrides: Dict[str, bool] = {}
    for pair in pairs:
        if "=" not in pair:
            raise ValueError(f"Option must be name=value: {pair}")
        key, raw = pair.split("=", 1)
        key = key.strip()
        if key not in DEFAULT_OPTIONS:
            raise ValueError(f"Unknown option: {key}")
        value = raw.strip().lower()
        if value in BOOL_TRUE:
            overrides[key] = True
        elif value in BOOL_FALSE:
            overrides[key] = False
        else:
            raise ValueError(f"Invalid value for {key}: {raw}")
    return normalize_options(overrides)


def cmake_cache_vars(options: Dict[str, bool], thirdparty_dir: str) -> Dict[str, str]:
    opts = normalize_options(options)

    def onoff(value: bool) -> str:
        return "ON" if value else "OFF"

    cache: Dict[str, str] = {
        "3RDPARTY_DIR": thirdparty_dir,
        "BUILD_LIBRARY_TYPE": "Shared" if opts["shared"] else "Static",
        "BUILD_CPP_STANDARD": "C++23"
        if opts["cpp23"]
        else "C++20"
        if opts["cpp20"]
        else "C++17",
        "BUILD_OPT_PROFILE": "Production" if opts["optimized"] else "Default",
        "BUILD_USE_VCPKG": "OFF",
        "BUILD_MODULE_Draw": onoff(opts["tcl"] or opts["tk"]),
        "USE_TCL": onoff(opts["tcl"]),
        "USE_TK": onoff(opts["tk"]),
        "USE_FREETYPE": onoff(opts["freetype"]),
        "USE_FREEIMAGE": onoff(opts["freeimage"]),
        "USE_RAPIDJSON": onoff(opts["rapidjson"]),
        "USE_TBB": onoff(opts["tbb"]),
        "USE_VTK": onoff(opts["vtk"]),
        "USE_DRACO": onoff(opts["draco"]),
        "USE_FFMPEG": onoff(opts["ffmpeg"]),
        "USE_OPENVR": onoff(opts["openvr"]),
        "BUILD_GTEST": onoff(opts["gtest"]),
        "BUILD_USE_PCH": onoff(opts["pch"]),
        "USE_D3D": onoff(opts["d3d"]),
        "BUILD_DOC_Overview": onoff(opts["docs"]),
        "USE_MMGR_TYPE": "JEMALLOC" if opts["jemalloc"] else "NATIVE",
    }

    if opts["angle"]:
        cache["USE_GLES2"] = "ON"
        cache["USE_OPENGL"] = "OFF"
    else:
        cache["USE_GLES2"] = "OFF"
        cache["USE_OPENGL"] = onoff(opts["opengl"])

    return cache
