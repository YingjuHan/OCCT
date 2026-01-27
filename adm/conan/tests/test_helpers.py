import json
import tempfile
import unittest
from pathlib import Path

from adm.conan import helpers


class HelpersTests(unittest.TestCase):
    def test_write_text_creates_parent(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "a" / "b" / "file.txt"
            helpers.write_text(target, "hello")
            self.assertTrue(target.exists())
            self.assertEqual(target.read_text(encoding="utf-8"), "hello\n")

    def test_write_json_roundtrip(self) -> None:
        with tempfile.TemporaryDirectory() as tmp:
            target = Path(tmp) / "data.json"
            helpers.write_json(target, {"b": 2, "a": 1})
            data = json.loads(target.read_text(encoding="utf-8"))
            self.assertEqual(data, {"a": 1, "b": 2})


if __name__ == "__main__":
    unittest.main()
