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
