import unittest
from pathlib import Path

from adm.conan import context, options


class ContextTests(unittest.TestCase):
    def test_kit_from_profile(self) -> None:
        kit = context.kit_from_profile(Path("conan/profiles/windows-vs2022"))
        self.assertEqual(kit, "windows-vs2022")

    def test_conan_args_default_no_remote(self) -> None:
        features = options.normalize_features({})
        ctx = context.ConanContext(
            repo_root=Path("C:/repo"),
            kit="windows-vs2022",
            build_type="Release",
            profile=Path("C:/repo/conan/profiles/windows-vs2022"),
            features=features,
            allow_remote=False,
        )
        args = ctx.conan_install_args()
        self.assertIn("--no-remote", args)
        self.assertIn("-g", args)


if __name__ == "__main__":
    unittest.main()
