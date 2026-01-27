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
