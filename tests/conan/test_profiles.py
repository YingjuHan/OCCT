import unittest
from pathlib import Path


class ProfilesTests(unittest.TestCase):
    def test_profiles_exist(self) -> None:
        root = Path(__file__).resolve().parents[2]
        profiles = root / "conan" / "profiles"
        expected = [
            "windows-vs2022",
            "windows-vs2019",
            "windows-vs2017",
            "windows-mingw",
            "linux-gcc",
            "emsdk",
        ]
        for name in expected:
            self.assertTrue((profiles / name).is_file())


if __name__ == "__main__":
    unittest.main()
