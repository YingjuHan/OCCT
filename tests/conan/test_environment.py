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
