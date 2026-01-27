from __future__ import annotations

import argparse
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List

REPO_ROOT = Path(__file__).resolve().parents[2]
if str(REPO_ROOT) not in sys.path:
    sys.path.insert(0, str(REPO_ROOT))

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
    conan_args = ctx.conan_install_args()
    suffix = ".exe" if python_exe.suffix.lower() == ".exe" else ""
    conan_exe = python_exe.parent / f"conan{suffix}"
    conan_cmd = [str(conan_exe)] + conan_args[1:]
    subprocess.check_call(conan_cmd)
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
