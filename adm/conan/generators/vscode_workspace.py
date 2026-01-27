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
