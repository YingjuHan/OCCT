import unittest
from pathlib import Path

from adm.conan import bootstrap


class BootstrapTests(unittest.TestCase):
    def test_venv_python_path_windows(self) -> None:
        venv = Path("C:/repo/.venv-conan")
        exe = bootstrap.venv_python(venv, platform_name="Windows")
        self.assertTrue(str(exe).endswith("Scripts\\python.exe"))

    def test_build_reexec_args(self) -> None:
        args = bootstrap.build_reexec_args(Path("C:/py.exe"), ["script.py", "--flag"])
        self.assertEqual(args[0], "C:/py.exe")
        self.assertEqual(args[1:], ["script.py", "--flag"])

    def test_should_reexec(self) -> None:
        target = Path("C:/repo/.venv-conan/Scripts/python.exe")
        current = Path("C:/python.exe")
        self.assertTrue(bootstrap.should_reexec(target, current))
        self.assertFalse(bootstrap.should_reexec(current, current))


if __name__ == "__main__":
    unittest.main()
