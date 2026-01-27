import tempfile
import unittest
from pathlib import Path

from adm.conan import context


class ContextTests(unittest.TestCase):
    def test_default_context_paths(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            ctx = context.build_context(["--kit", "test-kit"], root_dir=root)
            self.assertEqual(ctx.build_root, root / "build" / "test-kit")
            self.assertEqual(ctx.workspace_file, root / "build" / "test-kit" / "test-kit.code-workspace")

    def test_configs_from_args(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            ctx = context.build_context(
                ["--kit", "k", "--config", "RelWithDebInfo", "--config", "Debug"],
                root_dir=root,
            )
            self.assertEqual(ctx.configs, ["RelWithDebInfo", "Debug"])


if __name__ == "__main__":
    unittest.main()
