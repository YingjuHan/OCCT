from __future__ import annotations

import argparse
import platform
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable, List

from adm.conan import options
from adm.conan import profiles

DEFAULT_CONFIGS = ["Debug", "Release"]


def default_profile_name() -> str:
    system = platform.system().lower()
    if system.startswith("win"):
        return "vs2022"
    if system == "linux":
        return "linux-gcc"
    return "linux-gcc"


def _normalize_configs(values: Iterable[str] | None) -> List[str]:
    if not values:
        return list(DEFAULT_CONFIGS)
    configs: List[str] = []
    for value in values:
        for item in value.split(","):
            item = item.strip()
            if item:
                configs.append(item)
    return configs or list(DEFAULT_CONFIGS)


@dataclass(frozen=True)
class ConanContext:
    root_dir: Path
    kit: str
    profile: Path
    configs: List[str]
    options: dict[str, bool]
    allow_remote: bool
    clean: bool

    @property
    def build_root(self) -> Path:
        return self.root_dir / "build" / self.kit

    @property
    def conan_home(self) -> Path:
        return self.build_root / ".conan"

    @property
    def conanfile(self) -> Path:
        return self.root_dir / "conanfile.py"

    @property
    def thirdparty_dir(self) -> Path:
        return self.root_dir / "3rdparty-package"

    @property
    def workspace_file(self) -> Path:
        return self.build_root / f"{self.kit}.code-workspace"

    def build_dir(self, config: str) -> Path:
        return self.build_root / config


def parse_args(argv: List[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Generate OCCT Conan VSCode workspace")
    parser.add_argument("--kit", default="default", help="CMake kit name")
    parser.add_argument("--profile", default=None, help="Conan profile name or path")
    parser.add_argument("--config", dest="configs", action="append", help="Build config")
    parser.add_argument("--option", dest="options", action="append", default=[], help="Conan option name=value")
    parser.add_argument("--allow-remote", action="store_true", help="Allow Conan remotes")
    parser.add_argument("--clean", action="store_true", help="Remove build dir and clean project outputs")
    return parser.parse_args(argv)


def build_context(argv: List[str] | None = None, root_dir: Path | None = None) -> ConanContext:
    args = parse_args(argv)
    root = root_dir or Path(__file__).resolve().parents[2]
    profile_name = args.profile or default_profile_name()
    profile_path = profiles.resolve_profile_path(root, profile_name)
    configs = _normalize_configs(args.configs)
    opts = options.parse_option_pairs(args.options)
    if args.clean:
        opts["clean"] = True
    return ConanContext(
        root_dir=root,
        kit=args.kit,
        profile=profile_path,
        configs=configs,
        options=opts,
        allow_remote=args.allow_remote,
        clean=args.clean,
    )
