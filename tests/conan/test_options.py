import unittest

from adm.conan import options


class OptionsTests(unittest.TestCase):
    def test_defaults_include_expected_features(self) -> None:
        features = options.normalize_features({})
        self.assertTrue(features["tcl"])
        self.assertFalse(features["tcltk"])
        self.assertTrue(features["freetype"])
        self.assertTrue(features["opengl"])

    def test_unknown_feature_raises(self) -> None:
        with self.assertRaises(ValueError):
            options.normalize_features({"not-a-feature": True})

    def test_cmake_cache_mapping_draw_and_opengl(self) -> None:
        features = options.normalize_features(
            {"tcl": True, "tcltk": True, "angle": False, "opengl": True, "freetype": False}
        )
        cache = options.cmake_cache_from_features(features)
        self.assertTrue(cache["USE_TCL"])
        self.assertTrue(cache["USE_TK"])
        self.assertTrue(cache["BUILD_MODULE_Draw"])
        self.assertFalse(cache["USE_FREETYPE"])
        self.assertTrue(cache["USE_OPENGL"])
        self.assertFalse(cache["USE_GLES2"])

    def test_angle_disables_opengl(self) -> None:
        features = options.normalize_features({"angle": True, "opengl": True})
        cache = options.cmake_cache_from_features(features)
        self.assertTrue(cache["USE_GLES2"])
        self.assertFalse(cache["USE_OPENGL"])


if __name__ == "__main__":
    unittest.main()
