# OCCT Conan VSCode Workspace Design

## Goal
- Use Conan to provision OCCT 3rdparty dependencies with a default cache-only policy.
- Generate a VS Code workspace and CMake presets in one command for CMake Tools.
- Mirror vcpkg feature toggles and OCCT CMake defaults.
- Support Windows (VS2017/2019/2022, MinGW), Linux GCC, and Emscripten.

## Entry Point
- `python adm/conan/generate_vscode_workspace.py` (standalone script under `adm/conan`).
- Bootstraps Conan into `.venv-conan` when missing and re-execs under that interpreter.

## Layout
- `adm/conan/`: bootstrap, context, presets, VS Code helpers, Conan generators.
- `conanfile.py`: repository root, dependency-only recipe.
- `conan/profiles/*`: VS2017/2019/2022, MinGW, Linux GCC, emsdk.
- `build/<kit>/<cfg>/`: Conan output and build folders.
- `3rdparty-package/`: Conan imports to satisfy OCCT 3RDPARTY_DIR discovery.

## Conan Flow
- `ConanContext` parses args, installs custom generators, and writes a normalized profile to the build dir.
- `conan install` runs with `CMakeToolchain`, `CMakeDeps`, and custom generators.
- Default behavior is `--no-remote`; `--allow-remote` overrides.

## Generators
- `vscode_workspace`: emits `<build>/<kit>/<kit>.code-workspace` with preset names and minimal settings.
- `environment`: emits `environment.{sh,ps1,bat}.env` and activate scripts for runtime PATH setup.
- `project_cleaner`: optional cleanup of cache/presets when requested by recipe options.

## CMake Presets
- Generate `CMakeUserPresets.json` using Conan toolchain output, one preset per build type.
- Set cache variables such as `3RDPARTY_DIR=<repo>/3rdparty-package`, `BUILD_LIBRARY_TYPE`, and feature toggles.
- Prefer inheriting Conan-generated presets when present; otherwise set the toolchain directly.

## conanfile Options
- Options map to vcpkg features: tcl, tk, freetype, freeimage, rapidjson, tbb, vtk, draco,
  ffmpeg, openvr, jemalloc, gtest, docs, opengl, angle, d3d, pch, cpp20/cpp23, optimized.
- Defaults follow OCCT CMake defaults and vcpkg portfile behavior.

## Imports Strategy
- Copy Conan packages into `3rdparty-package/<alias>` to satisfy OCCT 3rdparty macros.
- Create stable alias names (e.g., `tcl`, `tk`, `freetype`, `tbb`) to match OCCT search patterns.

## Testing
- Unit tests cover preset generation and workspace path/preset naming without requiring Conan.
