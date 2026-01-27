import unittest


class ImportTests(unittest.TestCase):
    def test_adm_conan_imports(self) -> None:
        import adm.conan  # noqa: F401


if __name__ == "__main__":
    unittest.main()
