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
