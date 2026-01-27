import subprocess
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from adm.conan import generate_vscode_workspace as cli


class CliTests(unittest.TestCase):
    def test_parse_args_enable_disable(self) -> None:
        args = cli.parse_args(["--profile", "conan/profiles/windows-vs2022", "--enable", "tcl", "--disable", "tbb"])
        self.assertTrue(args.features["tcl"])
        self.assertFalse(args.features["tbb"])

    def test_script_help_runs(self) -> None:
        script = Path(__file__).resolve().parents[2] / "adm" / "conan" / "generate_vscode_workspace.py"
        result = subprocess.run([sys.executable, str(script), "--help"], capture_output=True, text=True)
        self.assertEqual(result.returncode, 0, result.stderr)

    @mock.patch("adm.conan.generate_vscode_workspace.bootstrap.ensure_conan_venv")
    @mock.patch("adm.conan.generate_vscode_workspace.bootstrap.reexec_if_needed")
    @mock.patch("adm.conan.generate_vscode_workspace.subprocess.check_call")
    def test_run_calls_conan(self, check_call, reexec_if_needed, ensure_conan_venv) -> None:
        args = cli.parse_args(["--profile", "conan/profiles/windows-vs2022"])
        ensure_conan_venv.return_value = Path("C:/repo/.venv-conan/Scripts/python.exe")
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            args.repo_root = root
            args.profile = root / "conan" / "profiles" / "windows-vs2022"
            cli.run(args)
        self.assertTrue(check_call.called)
        called = check_call.call_args[0][0]
        expected = ensure_conan_venv.return_value.parent / "conan.exe"
        self.assertEqual(called[0], str(expected))
        self.assertEqual(called[1], "install")


if __name__ == "__main__":
    unittest.main()
