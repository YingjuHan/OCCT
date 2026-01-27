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
