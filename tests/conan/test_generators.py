import unittest


class GeneratorImportTests(unittest.TestCase):
    def test_generators_import(self) -> None:
        import adm.conan.generators.environment as env_gen
        import adm.conan.generators.vscode_workspace as ws_gen
        self.assertTrue(hasattr(env_gen, "EnvironmentGenerator"))
        self.assertTrue(hasattr(ws_gen, "VSCodeWorkspace"))


if __name__ == "__main__":
    unittest.main()
