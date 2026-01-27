import unittest

from adm.conan import options


class OptionsTests(unittest.TestCase):
    def test_parse_option_pairs(self) -> None:
        opts = options.parse_option_pairs(["tbb=True", "shared=False"])
        self.assertTrue(opts["tbb"])
        self.assertFalse(opts["shared"])

    def test_normalize_enables_tcl(self) -> None:
        opts = options.normalize_options({"tk": True})
        self.assertTrue(opts["tcl"])

    def test_cmake_cache_angle_disables_opengl(self) -> None:
        opts = options.normalize_options({"angle": True, "opengl": True})
        cache = options.cmake_cache_vars(opts, "C:/thirdparty")
        self.assertEqual(cache["USE_GLES2"], "ON")
        self.assertEqual(cache["USE_OPENGL"], "OFF")


if __name__ == "__main__":
    unittest.main()
