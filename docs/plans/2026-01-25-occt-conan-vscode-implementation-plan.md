# OCCT Conan VSCode Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Provide a Conan-driven dependency workflow that generates VS Code workspace files and CMake user presets from `adm/conan/generate_vscode_workspace.py`, with cache-only default behavior and multi-platform profiles.

**Architecture:** Add a small Python toolchain under `adm/conan` that normalizes feature flags, builds Conan install arguments, and writes `CMakeUserPresets.json` plus a `<kit>.code-workspace`. Conan runs with custom generators to emit runtime environment files and copies dependencies into `3rdparty-package/` for OCCT discovery.

**Tech Stack:** Python 3 standard library, Conan 2, CMake Presets, VS Code CMake Tools.

---

**Skills:** @executing-plans @test-driven-development @systematic-debugging @verification-before-completion @using-git-worktrees @subagent-driven-development

### Task 1: Python Package and Test Scaffolding

**Files:**
- Create: `adm/__init__.py`
- Create: `adm/conan/__init__.py`
- Create: `adm/conan/generators/__init__.py`
- Create: `tests/__init__.py`
- Create: `tests/conan/__init__.py`
- Create: `tests/conan/test_imports.py`

**Step 1: Write the failing test**

```python
import unittest


class ImportTests(unittest.TestCase):
    def test_adm_conan_imports(self) -> None:
        import adm.conan  # noqa: F401


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_imports -v`
Expected: FAIL with `ModuleNotFoundError: No module named 'adm'`.

**Step 3: Write minimal implementation**

`adm/__init__.py`
```python
"""OCCT admin tooling package."""
```

`adm/conan/__init__.py`
```python
"""Conan tooling for OCCT."""
```

`adm/conan/generators/__init__.py`
```python
"""Custom Conan generators for OCCT."""
```

`tests/__init__.py`
```python
```

`tests/conan/__init__.py`
```python
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_imports -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/__init__.py adm/conan/__init__.py adm/conan/generators/__init__.py tests/__init__.py tests/conan/__init__.py tests/conan/test_imports.py
git commit -m "test: add basic adm.conan import scaffolding"
```

### Task 2: Feature Options and CMake Cache Mapping

**Files:**
- Create: `adm/conan/options.py`
- Create: `tests/conan/test_options.py`

**Step 1: Write the failing test**

```python
import unittest

from adm.conan import options


class OptionsTests(unittest.TestCase):
    def test_defaults_include_expected_features(self) -> None:
        features = options.normalize_features({})
        self.assertTrue(features["tcl"])
        self.assertFalse(features["tcltk"])
        self.assertTrue(features["freetype"])
        self.assertTrue(features["opengl"])

    def test_unknown_feature_raises(self) -> None:
        with self.assertRaises(ValueError):
            options.normalize_features({"not-a-feature": True})

    def test_cmake_cache_mapping_draw_and_opengl(self) -> None:
        features = options.normalize_features(
            {"tcl": True, "tcltk": True, "angle": False, "opengl": True, "freetype": False}
        )
        cache = options.cmake_cache_from_features(features)
        self.assertTrue(cache["USE_TCL"])
        self.assertTrue(cache["USE_TK"])
        self.assertTrue(cache["BUILD_MODULE_Draw"])
        self.assertFalse(cache["USE_FREETYPE"])
        self.assertTrue(cache["USE_OPENGL"])
        self.assertFalse(cache["USE_GLES2"])

    def test_angle_disables_opengl(self) -> None:
        features = options.normalize_features({"angle": True, "opengl": True})
        cache = options.cmake_cache_from_features(features)
        self.assertTrue(cache["USE_GLES2"])
        self.assertFalse(cache["USE_OPENGL"])


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_options -v`
Expected: FAIL with `ImportError` for `adm.conan.options`.

**Step 3: Write minimal implementation**

`adm/conan/options.py`
```python
from __future__ import annotations

from typing import Dict

FEATURE_DEFAULTS: Dict[str, bool] = {
    "angle": False,
    "opengl": True,
    "tcl": True,
    "tcltk": False,
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
    "pch": False,
    "cpp20": False,
    "cpp23": False,
    "optimized": False,
    "d3d": False,
}

FEATURE_ALIASES = {
    "tk": "tcltk",
}


def normalize_features(overrides: Dict[str, bool] | None) -> Dict[str, bool]:
    features = dict(FEATURE_DEFAULTS)
    if not overrides:
        return features
    for key, value in overrides.items():
        key = FEATURE_ALIASES.get(key, key)
        if key not in features:
            raise ValueError(f"Unknown feature: {key}")
        features[key] = bool(value)
    if features.get("tcltk"):
        features["tcl"] = True
    return features


def cmake_cache_from_features(features: Dict[str, bool]) -> Dict[str, object]:
    cache: Dict[str, object] = {
        "USE_TCL": bool(features["tcl"]),
        "USE_TK": bool(features["tcltk"]),
        "BUILD_MODULE_Draw": bool(features["tcl"] or features["tcltk"]),
        "USE_FREETYPE": bool(features["freetype"]),
        "USE_FREEIMAGE": bool(features["freeimage"]),
        "USE_RAPIDJSON": bool(features["rapidjson"]),
        "USE_TBB": bool(features["tbb"]),
        "USE_VTK": bool(features["vtk"]),
        "USE_DRACO": bool(features["draco"]),
        "USE_FFMPEG": bool(features["ffmpeg"]),
        "USE_OPENVR": bool(features["openvr"]),
        "BUILD_GTEST": bool(features["gtest"]),
        "BUILD_USE_PCH": bool(features["pch"]),
        "USE_D3D": bool(features["d3d"]),
        "BUILD_DOC_Overview": bool(features["docs"]),
    }

    if features["angle"]:
        cache["USE_GLES2"] = True
        cache["USE_OPENGL"] = False
    else:
        cache["USE_OPENGL"] = bool(features["opengl"])
        cache["USE_GLES2"] = False

    if features["cpp23"]:
        cache["BUILD_CPP_STANDARD"] = "C++23"
    elif features["cpp20"]:
        cache["BUILD_CPP_STANDARD"] = "C++20"
    else:
        cache["BUILD_CPP_STANDARD"] = "C++17"

    cache["BUILD_OPT_PROFILE"] = "Production" if features["optimized"] else "Default"
    cache["USE_MMGR_TYPE"] = "JEMALLOC" if features["jemalloc"] else "NATIVE"
    return cache


def conan_options_from_features(features: Dict[str, bool]) -> Dict[str, bool]:
    return {name: bool(features[name]) for name in sorted(FEATURE_DEFAULTS.keys())}
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_options -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/options.py tests/conan/test_options.py
git commit -m "feat: add conan feature normalization and cmake mapping"
```
### Task 3: Repo Config Helpers and Version Parsing

**Files:**
- Create: `adm/conan/config.py`
- Create: `tests/conan/test_config.py`

**Step 1: Write the failing test**

```python
import unittest
from pathlib import Path

from adm.conan import config


class ConfigTests(unittest.TestCase):
    def test_parse_version_cmake(self) -> None:
        sample = (
            "set (OCC_VERSION_MAJOR 8 )\n"
            "set (OCC_VERSION_MINOR 0 )\n"
            "set (OCC_VERSION_MAINTENANCE 0 )\n"
            "set (OCC_VERSION_DEVELOPMENT \"rc3\" )\n"
        )
        version = config.parse_version_cmake(sample)
        self.assertEqual(version, (8, 0, 0, "rc3"))
        self.assertEqual(config.version_string(version), "8.0.0-rc3")

    def test_repo_root_contains_cmake(self) -> None:
        root = config.repo_root()
        self.assertTrue((root / "CMakeLists.txt").is_file())


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_config -v`
Expected: FAIL with `ImportError` for `adm.conan.config`.

**Step 3: Write minimal implementation**

`adm/conan/config.py`
```python
from __future__ import annotations

import re
from pathlib import Path
from typing import Tuple

_VERSION_PATTERN = re.compile(r"set\\s*\\(OCC_VERSION_(MAJOR|MINOR|MAINTENANCE|DEVELOPMENT)\\s+([^\\s\\)]+)")


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
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_config -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/config.py tests/conan/test_config.py
git commit -m "feat: add config helpers and version parsing"
```

### Task 4: CMake User Presets Writer

**Files:**
- Create: `adm/conan/presets.py`
- Create: `tests/conan/test_presets.py`

**Step 1: Write the failing test**

```python
import json
import tempfile
import unittest
from pathlib import Path

from adm.conan import presets


class PresetsTests(unittest.TestCase):
    def test_render_user_presets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            build_root = root / "build"
            toolchain = build_root / "kit" / "Release" / "conan_toolchain.cmake"
            data = presets.render_user_presets(
                source_dir=root,
                build_root=build_root,
                kit="kit",
                build_types=["Release"],
                cache_vars={"USE_TCL": True, "3RDPARTY_DIR": str(root / "3rdparty-package")},
                toolchain_file=toolchain,
            )
            configure = data["configurePresets"][0]
            self.assertEqual(configure["name"], "kit-release")
            self.assertEqual(configure["binaryDir"], str(build_root / "kit" / "Release"))
            self.assertEqual(configure["toolchainFile"], str(toolchain))
            self.assertTrue(configure["cacheVariables"]["USE_TCL"])

    def test_write_user_presets(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            output = root / "CMakeUserPresets.json"
            presets.write_user_presets(output, {"version": 4})
            data = json.loads(output.read_text(encoding="utf-8"))
            self.assertEqual(data["version"], 4)


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_presets -v`
Expected: FAIL with `ImportError` for `adm.conan.presets`.

**Step 3: Write minimal implementation**

`adm/conan/presets.py`
```python
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
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_presets -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/presets.py tests/conan/test_presets.py
git commit -m "feat: add cmake user presets generator"
```
### Task 5: VS Code Workspace Writer

**Files:**
- Create: `adm/conan/vscode.py`
- Create: `tests/conan/test_vscode.py`

**Step 1: Write the failing test**

```python
import json
import tempfile
import unittest
from pathlib import Path

from adm.conan import vscode


class VSCodeTests(unittest.TestCase):
    def test_render_workspace(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            workspace = vscode.render_workspace(root, root / "build", "win-x64", "Release")
            settings = workspace["settings"]
            self.assertEqual(settings["cmake.useCMakePresets"], "always")
            self.assertEqual(settings["cmake.configurePreset"], "win-x64-release")

    def test_write_workspace(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            workspace_dir = root / "build" / "kit"
            workspace_dir.mkdir(parents=True, exist_ok=True)
            path = workspace_dir / "kit.code-workspace"
            vscode.write_workspace(path, {"folders": []})
            data = json.loads(path.read_text(encoding="utf-8"))
            self.assertEqual(data["folders"], [])


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_vscode -v`
Expected: FAIL with `ImportError` for `adm.conan.vscode`.

**Step 3: Write minimal implementation**

`adm/conan/vscode.py`
```python
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
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_vscode -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/vscode.py tests/conan/test_vscode.py
git commit -m "feat: add vscode workspace generator"
```

### Task 6: Runtime Environment File Rendering

**Files:**
- Create: `adm/conan/environment.py`
- Create: `tests/conan/test_environment.py`

**Step 1: Write the failing test**

```python
import tempfile
import unittest
from pathlib import Path

from adm.conan import environment


class EnvironmentTests(unittest.TestCase):
    def test_render_env_windows(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            build_dir = Path(tmp) / "build"
            thirdparty = Path(tmp) / "3rdparty-package"
            env_files = environment.render_env_files(build_dir, thirdparty, platform_name="Windows")
            ps1 = env_files["environment.ps1.env"]
            self.assertIn(str(build_dir / "bin"), ps1)
            self.assertIn(str(thirdparty / "bin"), ps1)


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_environment -v`
Expected: FAIL with `ImportError` for `adm.conan.environment`.

**Step 3: Write minimal implementation**

`adm/conan/environment.py`
```python
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
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_environment -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/environment.py tests/conan/test_environment.py
git commit -m "feat: add runtime environment file renderer"
```
### Task 7: Conan Context and Argument Builder

**Files:**
- Create: `adm/conan/context.py`
- Create: `tests/conan/test_context.py`

**Step 1: Write the failing test**

```python
import unittest
from pathlib import Path

from adm.conan import context, options


class ContextTests(unittest.TestCase):
    def test_kit_from_profile(self) -> None:
        kit = context.kit_from_profile(Path("conan/profiles/windows-vs2022"))
        self.assertEqual(kit, "windows-vs2022")

    def test_conan_args_default_no_remote(self) -> None:
        features = options.normalize_features({})
        ctx = context.ConanContext(
            repo_root=Path("C:/repo"),
            kit="windows-vs2022",
            build_type="Release",
            profile=Path("C:/repo/conan/profiles/windows-vs2022"),
            features=features,
            allow_remote=False,
        )
        args = ctx.conan_install_args()
        self.assertIn("--no-remote", args)
        self.assertIn("-g", args)


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_context -v`
Expected: FAIL with `ImportError` for `adm.conan.context`.

**Step 3: Write minimal implementation**

`adm/conan/context.py`
```python
from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

from adm.conan import options


def kit_from_profile(profile: Path) -> str:
    return profile.stem


@dataclass
class ConanContext:
    repo_root: Path
    kit: str
    build_type: str
    profile: Path
    features: Dict[str, bool]
    allow_remote: bool

    @property
    def build_dir(self) -> Path:
        return self.repo_root / "build" / self.kit / self.build_type

    @property
    def generators_dir(self) -> Path:
        return self.build_dir / "generators"

    @property
    def toolchain_file(self) -> Path:
        return self.build_dir / "conan_toolchain.cmake"

    def conan_install_args(self) -> List[str]:
        args = [
            "conan",
            "install",
            str(self.repo_root),
            "-pr:h",
            str(self.profile),
            "-pr:b",
            str(self.profile),
            "-s:h",
            f"build_type={self.build_type}",
            "--output-folder",
            str(self.build_dir),
            "--build=missing",
            "-g",
            "CMakeDeps",
            "-g",
            "CMakeToolchain",
            "-g",
            "EnvironmentGenerator",
            "-g",
            "VSCodeWorkspace",
        ]
        if not self.allow_remote:
            args.append("--no-remote")
        for name, value in options.conan_options_from_features(self.features).items():
            args.extend(["-o", f"{name}={str(value)}"])
        return args
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_context -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/context.py tests/conan/test_context.py
git commit -m "feat: add conan context and install args"
```

### Task 8: Conan Bootstrap Helpers

**Files:**
- Create: `adm/conan/bootstrap.py`
- Create: `tests/conan/test_bootstrap.py`

**Step 1: Write the failing test**

```python
import unittest
from pathlib import Path

from adm.conan import bootstrap


class BootstrapTests(unittest.TestCase):
    def test_venv_python_path_windows(self) -> None:
        venv = Path("C:/repo/.venv-conan")
        exe = bootstrap.venv_python(venv, platform_name="Windows")
        self.assertTrue(str(exe).endswith("Scripts\\python.exe"))

    def test_build_reexec_args(self) -> None:
        args = bootstrap.build_reexec_args(Path("C:/py.exe"), ["script.py", "--flag"])
        self.assertEqual(args[0], "C:/py.exe")
        self.assertEqual(args[1:], ["script.py", "--flag"])

    def test_should_reexec(self) -> None:
        target = Path("C:/repo/.venv-conan/Scripts/python.exe")
        current = Path("C:/python.exe")
        self.assertTrue(bootstrap.should_reexec(target, current))
        self.assertFalse(bootstrap.should_reexec(current, current))


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_bootstrap -v`
Expected: FAIL with `ImportError` for `adm.conan.bootstrap`.

**Step 3: Write minimal implementation**

`adm/conan/bootstrap.py`
```python
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
    return [str(python_exe)] + list(argv)


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
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_bootstrap -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/bootstrap.py tests/conan/test_bootstrap.py
git commit -m "feat: add conan bootstrap helpers"
```
### Task 9: CLI Entry Point and Orchestration

**Files:**
- Create: `adm/conan/generate_vscode_workspace.py`
- Create: `tests/conan/test_cli.py`
- Modify: `adm/conan/context.py`

**Step 1: Write the failing test**

```python
import unittest
from pathlib import Path
from unittest import mock

from adm.conan import generate_vscode_workspace as cli


class CliTests(unittest.TestCase):
    def test_parse_args_enable_disable(self) -> None:
        args = cli.parse_args(["--profile", "conan/profiles/windows-vs2022", "--enable", "tcl", "--disable", "tbb"])
        self.assertTrue(args.features["tcl"])
        self.assertFalse(args.features["tbb"])

    @mock.patch("adm.conan.generate_vscode_workspace.bootstrap.reexec_if_needed")
    @mock.patch("adm.conan.generate_vscode_workspace.subprocess.check_call")
    def test_run_calls_conan(self, check_call, reexec_if_needed) -> None:
        args = cli.parse_args(["--profile", "conan/profiles/windows-vs2022"])
        args.repo_root = Path("C:/repo")
        cli.run(args)
        self.assertTrue(check_call.called)


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_cli -v`
Expected: FAIL with `ImportError` for `adm.conan.generate_vscode_workspace`.

**Step 3: Write minimal implementation**

`adm/conan/generate_vscode_workspace.py`
```python
from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

from adm.conan import bootstrap, config, context, options, presets, vscode


@dataclass
class Args:
    repo_root: Path
    profile: Path
    build_type: str
    kit: str
    allow_remote: bool
    features: Dict[str, bool]


def parse_args(argv: List[str]) -> Args:
    parser = argparse.ArgumentParser(description="Generate OCCT Conan VSCode workspace")
    parser.add_argument("--profile", required=True, help="Conan profile to use")
    parser.add_argument("--build-type", default="Release")
    parser.add_argument("--kit", default="")
    parser.add_argument("--allow-remote", action="store_true")
    parser.add_argument("--enable", nargs="*", default=[])
    parser.add_argument("--disable", nargs="*", default=[])

    ns = parser.parse_args(argv)
    repo_root = config.repo_root()
    profile = Path(ns.profile)
    kit = ns.kit or context.kit_from_profile(profile)
    overrides = {name: True for name in ns.enable}
    overrides.update({name: False for name in ns.disable})
    features = options.normalize_features(overrides)

    return Args(
        repo_root=repo_root,
        profile=profile,
        build_type=ns.build_type,
        kit=kit,
        allow_remote=ns.allow_remote,
        features=features,
    )


def run(args: Args) -> None:
    python_exe = bootstrap.ensure_conan_venv(args.repo_root)
    bootstrap.reexec_if_needed(python_exe)
    ctx = context.ConanContext(
        repo_root=args.repo_root,
        kit=args.kit,
        build_type=args.build_type,
        profile=args.profile,
        features=args.features,
        allow_remote=args.allow_remote,
    )
    subprocess.check_call(ctx.conan_install_args())
    cache_vars = options.cmake_cache_from_features(args.features)
    cache_vars["3RDPARTY_DIR"] = str(args.repo_root / "3rdparty-package")
    presets_path = args.repo_root / "CMakeUserPresets.json"
    user_presets = presets.render_user_presets(
        source_dir=args.repo_root,
        build_root=args.repo_root / "build",
        kit=args.kit,
        build_types=[args.build_type],
        cache_vars=cache_vars,
        toolchain_file=ctx.toolchain_file,
    )
    presets.write_user_presets(presets_path, user_presets)
    workspace_dir = args.repo_root / "build" / args.kit
    workspace_path = workspace_dir / f"{args.kit}.code-workspace"
    workspace = vscode.render_workspace(args.repo_root, args.repo_root / "build", args.kit, args.build_type)
    workspace_dir.mkdir(parents=True, exist_ok=True)
    vscode.write_workspace(workspace_path, workspace)


def main(argv: List[str] | None = None) -> int:
    args = parse_args(argv or sys.argv[1:])
    run(args)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_cli -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/generate_vscode_workspace.py tests/conan/test_cli.py adm/conan/context.py
git commit -m "feat: add conan vscode workspace cli"
```
### Task 10: Conanfile, Profiles, and Git Ignore Updates

**Files:**
- Create: `conanfile.py`
- Create: `conan/profiles/windows-vs2022`
- Create: `conan/profiles/windows-vs2019`
- Create: `conan/profiles/windows-vs2017`
- Create: `conan/profiles/windows-mingw`
- Create: `conan/profiles/linux-gcc`
- Create: `conan/profiles/emsdk`
- Create: `tests/conan/test_profiles.py`
- Modify: `.gitignore`

**Step 1: Write the failing test**

```python
import unittest
from pathlib import Path


class ProfilesTests(unittest.TestCase):
    def test_profiles_exist(self) -> None:
        root = Path(__file__).resolve().parents[2]
        profiles = root / "conan" / "profiles"
        expected = [
            "windows-vs2022",
            "windows-vs2019",
            "windows-vs2017",
            "windows-mingw",
            "linux-gcc",
            "emsdk",
        ]
        for name in expected:
            self.assertTrue((profiles / name).is_file())


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_profiles -v`
Expected: FAIL with missing profile files.

**Step 3: Write minimal implementation**

`conanfile.py`
```python
from __future__ import annotations

from pathlib import Path

from conan import ConanFile

from adm.conan import config


class OcctConanFile(ConanFile):
    name = "occt"
    version = "0.0.0"
    settings = "os", "arch", "compiler", "build_type"
    generators = ("CMakeToolchain", "CMakeDeps")

    options = {
        "angle": [True, False],
        "opengl": [True, False],
        "tcl": [True, False],
        "tcltk": [True, False],
        "freetype": [True, False],
        "freeimage": [True, False],
        "rapidjson": [True, False],
        "tbb": [True, False],
        "vtk": [True, False],
        "draco": [True, False],
        "ffmpeg": [True, False],
        "openvr": [True, False],
        "jemalloc": [True, False],
        "gtest": [True, False],
        "docs": [True, False],
        "pch": [True, False],
        "cpp20": [True, False],
        "cpp23": [True, False],
        "optimized": [True, False],
        "d3d": [True, False],
    }

    default_options = {
        "angle": False,
        "opengl": True,
        "tcl": True,
        "tcltk": False,
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
        "pch": False,
        "cpp20": False,
        "cpp23": False,
        "optimized": False,
        "d3d": False,
    }

    def configure(self) -> None:
        self.version = config.read_version(Path(self.recipe_folder))
        if self.options.tcltk:
            self.options.tcl = True

    def requirements(self) -> None:
        if self.options.tcl:
            self.requires("tcl/8.6.14")
        if self.options.tcltk:
            self.requires("tcl/8.6.14", options={"tk": True})
        if self.options.freetype:
            self.requires("freetype/2.13.2", options={"with_brotli": False, "with_bzip2": False})
        if self.options.freeimage:
            self.requires("freeimage/3.18.0")
        if self.options.rapidjson:
            self.requires("rapidjson/cci.20230929")
        if self.options.tbb:
            self.requires("tbb/2021.12.0")
        if self.options.vtk:
            self.requires("vtk/9.3.0", options={"opengl": True})
        if self.options.draco:
            self.requires("draco/1.5.7")
        if self.options.ffmpeg:
            self.requires("ffmpeg/6.1.1")
        if self.options.openvr:
            self.requires("openvr/1.26.7")
        if self.options.jemalloc:
            self.requires("jemalloc/5.3.0")
        if self.options.gtest:
            self.requires("gtest/1.14.0")
        if self.options.docs:
            self.requires("doxygen/1.10.0")

    def build(self) -> None:
        pass

    def package(self) -> None:
        pass

    def package_info(self) -> None:
        pass
```

`conan/profiles/windows-vs2022`
```
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=193
compiler.runtime=dynamic
build_type=Release
```

`conan/profiles/windows-vs2019`
```
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=192
compiler.runtime=dynamic
build_type=Release
```

`conan/profiles/windows-vs2017`
```
[settings]
os=Windows
arch=x86_64
compiler=msvc
compiler.version=191
compiler.runtime=dynamic
build_type=Release
```

`conan/profiles/windows-mingw`
```
[settings]
os=Windows
arch=x86_64
compiler=gcc
compiler.version=13
compiler.libcxx=libstdc++11
compiler.threads=posix
compiler.exception=seh
build_type=Release
```

`conan/profiles/linux-gcc`
```
[settings]
os=Linux
arch=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Release
```

`conan/profiles/emsdk`
```
[settings]
os=Emscripten
arch=wasm32
compiler=clang
compiler.version=16
compiler.libcxx=libc++
build_type=Release
```

`.gitignore`
```text
!/conan/
!/conanfile.py
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_profiles -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add conanfile.py conan/profiles/windows-vs2022 conan/profiles/windows-vs2019 conan/profiles/windows-vs2017 conan/profiles/windows-mingw conan/profiles/linux-gcc conan/profiles/emsdk tests/conan/test_profiles.py .gitignore
git commit -m "feat: add conanfile, profiles, and gitignore allowlist"
```

### Task 11: Smoke Test the Generator (Manual)

**Files:**
- None (manual verification)

**Step 1: Run generator**

Run: `python adm/conan/generate_vscode_workspace.py --profile conan/profiles/windows-vs2022`
Expected: `CMakeUserPresets.json` created at repo root and `<kit>.code-workspace` created under `build/windows-vs2022/`.

**Step 2: Verify outputs**

Run: `dir build\\windows-vs2022`
Expected: `windows-vs2022.code-workspace` exists.

**Step 3: Commit if any adjustments**

If manual changes are required, commit them with a short message.
### Task 12: Conan Generator Wrappers

**Files:**
- Create: `adm/conan/generators/environment.py`
- Create: `adm/conan/generators/vscode_workspace.py`
- Create: `tests/conan/test_generators.py`

**Step 1: Write the failing test**

```python
import unittest


class GeneratorImportTests(unittest.TestCase):
    def test_generators_import(self) -> None:
        import adm.conan.generators.environment as env_gen
        import adm.conan.generators.vscode_workspace as ws_gen
        self.assertTrue(hasattr(env_gen, "EnvironmentGenerator"))
        self.assertTrue(hasattr(ws_gen, "VSCodeWorkspace"))


if __name__ == "__main__":
    unittest.main()
```

**Step 2: Run test to verify it fails**

Run: `python -m unittest tests.conan.test_generators -v`
Expected: FAIL with `ImportError` for generator modules.

**Step 3: Write minimal implementation**

`adm/conan/generators/environment.py`
```python
from __future__ import annotations

import platform
from pathlib import Path

from adm.conan import environment

try:
    from conan.internal import check_duplicated_generator
except Exception:  # pragma: no cover - conan not installed
    def check_duplicated_generator(*_args, **_kwargs):
        return None


class EnvironmentGenerator(object):
    def __init__(self, conanfile):
        self._conanfile = conanfile

    def generate(self):
        check_duplicated_generator(self, self._conanfile)
        build_dir = Path(self._conanfile.build_folder)
        source_dir = Path(self._conanfile.source_folder)
        thirdparty_dir = source_dir / "3rdparty-package"
        env_files = environment.render_env_files(build_dir, thirdparty_dir, platform.system())
        act_files = environment.render_activate_files(platform.system())

        out_dir = Path(self._conanfile.generators_folder)
        out_dir.mkdir(parents=True, exist_ok=True)
        for name, content in env_files.items():
            (out_dir / name).write_text(content, encoding="utf-8")
        for name, content in act_files.items():
            (out_dir / name).write_text(content, encoding="utf-8")
```

`adm/conan/generators/vscode_workspace.py`
```python
from __future__ import annotations

from pathlib import Path

from adm.conan import vscode

try:
    from conan.internal import check_duplicated_generator
except Exception:  # pragma: no cover - conan not installed
    def check_duplicated_generator(*_args, **_kwargs):
        return None


class VSCodeWorkspace(object):
    def __init__(self, conanfile):
        self._conanfile = conanfile

    def generate(self):
        check_duplicated_generator(self, self._conanfile)
        source_dir = Path(self._conanfile.source_folder)
        build_dir = Path(self._conanfile.build_folder)
        kit = build_dir.parent.name
        build_type = str(self._conanfile.settings.get_safe("build_type") or build_dir.name or "Release")
        workspace_dir = build_dir.parent
        workspace_path = workspace_dir / f"{kit}.code-workspace"
        workspace = vscode.render_workspace(source_dir, source_dir / "build", kit, build_type)
        workspace_dir.mkdir(parents=True, exist_ok=True)
        vscode.write_workspace(workspace_path, workspace)
```

**Step 4: Run test to verify it passes**

Run: `python -m unittest tests.conan.test_generators -v`
Expected: PASS.

**Step 5: Commit**

```bash
git add adm/conan/generators/environment.py adm/conan/generators/vscode_workspace.py tests/conan/test_generators.py
git commit -m "feat: add conan generator wrappers"
```
