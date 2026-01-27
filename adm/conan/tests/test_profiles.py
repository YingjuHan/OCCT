import tempfile
import unittest
from pathlib import Path

from adm.conan import profiles


class ProfilesTests(unittest.TestCase):
    def test_normalize_profile_inlines_includes(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            base = root / "base"
            child = root / "child"
            base.write_text("[settings]\nos=Windows\n", encoding="utf-8")
            child.write_text(f"include({base.name})\n[settings]\narch=x86_64\n", encoding="utf-8")
            output = root / "out.profile"
            profiles.normalize_profile(child, output)
            text = output.read_text(encoding="utf-8")
            self.assertIn("os=Windows", text)
            self.assertIn("arch=x86_64", text)
            self.assertNotIn("include(", text)


if __name__ == "__main__":
    unittest.main()
