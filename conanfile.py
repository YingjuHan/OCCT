from __future__ import annotations

from pathlib import Path

from conan import ConanFile

from adm.conan import config


class OcctConanFile(ConanFile):
    name = "occt"
    version = "0.0.0"
    settings = "os", "arch", "compiler", "build_type"
    generators = ("CMakeToolchain", "CMakeDeps")

    options = {
        "angle": [True, False],
        "opengl": [True, False],
        "tcl": [True, False],
        "tcltk": [True, False],
        "freetype": [True, False],
        "freeimage": [True, False],
        "rapidjson": [True, False],
        "tbb": [True, False],
        "vtk": [True, False],
        "draco": [True, False],
        "ffmpeg": [True, False],
        "openvr": [True, False],
        "jemalloc": [True, False],
        "gtest": [True, False],
        "docs": [True, False],
        "pch": [True, False],
        "cpp20": [True, False],
        "cpp23": [True, False],
        "optimized": [True, False],
        "d3d": [True, False],
    }

    default_options = {
        "angle": False,
        "opengl": True,
        "tcl": True,
        "tcltk": False,
        "freetype": True,
        "freeimage": False,
        "rapidjson": False,
        "tbb": False,
        "vtk": False,
        "draco": False,
        "ffmpeg": False,
        "openvr": False,
        "jemalloc": False,
        "gtest": False,
        "docs": False,
        "pch": False,
        "cpp20": False,
        "cpp23": False,
        "optimized": False,
        "d3d": False,
    }

    def configure(self) -> None:
        self.version = config.read_version(Path(self.recipe_folder))
        if self.options.tcltk:
            self.options.tcl = True

    def requirements(self) -> None:
        if self.options.tcl:
            self.requires("tcl/8.6.14")
        if self.options.tcltk:
            self.requires("tcl/8.6.14", options={"tk": True})
        if self.options.freetype:
            self.requires("freetype/2.13.2", options={"with_brotli": False, "with_bzip2": False})
        if self.options.freeimage:
            self.requires("freeimage/3.18.0")
        if self.options.rapidjson:
            self.requires("rapidjson/cci.20230929")
        if self.options.tbb:
            self.requires("tbb/2021.12.0")
        if self.options.vtk:
            self.requires("vtk/9.3.0", options={"opengl": True})
        if self.options.draco:
            self.requires("draco/1.5.7")
        if self.options.ffmpeg:
            self.requires("ffmpeg/6.1.1")
        if self.options.openvr:
            self.requires("openvr/1.26.7")
        if self.options.jemalloc:
            self.requires("jemalloc/5.3.0")
        if self.options.gtest:
            self.requires("gtest/1.14.0")
        if self.options.docs:
            self.requires("doxygen/1.10.0")

    def build(self) -> None:
        pass

    def package(self) -> None:
        pass

    def package_info(self) -> None:
        pass
