Changelog {#occt__changelog}
============================
This document describes changes between tagged OCCT versions.

Note that version numbers do not necessarily reflect the amount of changes between versions.
A version number reflects a release that is known to pass CI/CD tests,
and versions may be tagged more or less frequently at different times.

Not all changes are documented here - only major modifications are highlighted here.
To examine the full set of changes between versions, you can use git to browse the changes between the tags.

Please take a look onto [upgrade/upgrade.md](../upgrade/upgrade.md)
if you are upgrading your product to a newer version of OCCT.

OCCT 7.X (in development)
-----------------------------------------------------------------------------------------------------------------------

- **General**
   - Added missing class field initializers to many classes.
   - `Standard_Failure` now inherits `std::exception`, implements `std::exception::what()`;
     inheritance from `Standard_Transient` has been removed
     (application code catching messages from `Standard_Failure` might require modifications).
   - Adapted for C++ range-based loops: `TopExp_Explorer`, `TDF_ChildIterator` (`TDF_Label`), `NCollection_DataMap`,
     `TopoDS_Iterator` (`TopoDS_Shape`), `TColStd_PackedMapOfInteger`.
   - Linux: `Standard_ErrorHandler` now relies on `thread_local` stack instead of global mutex
     (improves performance in multi-threaded environments).
- **Modeling**
   - Improved stability of results produced by `BRepOffset`, `BRepFill_Evolved`
     (unordered maps replaced by ordered maps).
   - Bugfixes in `HLRBRep_PolyAlgo` for transformed shapes.
   - Back-ported bug-fixes from OCCT 7.8 and OCCT 7.9 releases.
- **Data Exchange**
   - Bugfixes for STL and OBJ formats.
   - Back-ported bug-fixes for STEP and IGES from OCCT 7.8 and OCCT 7.9 releases.
   - Back-ported STEP parser performance improvement.
- **Visualization**
   - HiDPI: transform-persistent objects (non-zoomable text, trihedrons, viewcube, etc.)
     are now automatically scaled based on `Graphic3d_RenderingParams::ResolutionRatio()`.
   - Enable wide lines within OpenGL Core Profile (when available).
   - Linux: introduced Wayland support.
   - `Aspect_DisplayConnection` is now virtual interface
     (use `Xw_DisplayConnection` for Xlib and `Wayland_DisplayConnection` for Wayland).
   - Redesigned `Image_AlienPixMap` - split into several implementations for different image libraries / formats
     (`Image_RWFreeImage` for FreeImage, `Image_RWWinCodec` for WinCodec)
     with base interface defined by `Image_RWPixMap`;
     `Image_AlienPixMap` provides the same interface as before, but now it works as a selector to different libraries;
     introduced `Image_RWPNG` for using `libpng`.
   - CJK: prefer "Microsoft Yahei" over legacy "SimSun" font on Windows; fixed rendering of punctuation symbols.
   - Fixed detection of (some) cylinders.
   - Fixed misdetection of degenerated line segments.
- **Configuration**
   - Tcl: fixed compatibility issues with *Tcl 9.0*.
   - MinGW: added `.dll.a` lookup and `.rc` embedding (DLL information);
   - MSVC: removed `#pragma comment(lib, "libX")` from code, fixed unescaped quotes within generated config file;
   - Linux: fixed auto-detection of system Draco library, added `-Wl,--no-undefined` and `-Wl,--as-needed` flags;
   - WebAssembly: fixed runtime issues within `WASM64` (`-sMEMORY64=1`), fixed building issues with Emscripten 4.x;
   - Common: removed redundant building macros,
     removed modification of `custom.sh` within INSTALL location during configure step.
- **Draw Harness**
   - Linux: fixed RED color propagated to the next command.
   - Linux: fixed hanging `wzoom` with AXO viewer.
   - Linux: added preliminary Wayland support (`vinit -wayland`) for 3D Viewer.
   - `Draw_Interpreter` initialization is postponed till the first usage;
     fixes errors inside Tcl library when application links to `TKDraw` but doesn't use Tcl.
   - Added commands `dumpjson` and `vdumpjson`.
   - Fixed double-inclusion of command in `help` listing.
   - Windows: 3D Viewer can be now created with `vinit -dpiaware` flag for HiDPI screens.
   - Windows: `XProgress +g` now displays progress within taskbar icon (`ITaskbarList3`).
- **Testing**
   - Moved (a major part of) non-public tests from `tests` to `tests_occ`.
   - Linux: fixed updating of `cpulimit`, improved process termination by `cpulimit`.
   - `testgrid` now prints progress details into console, may use GUI progress bat (`XProgress +g`),
     generates `summary.csv` (`testdiff` now generates `diff.csv`).
   - Test cases have been corrected to clean-up large file artifacts.
- **Debugging**
   - Improved Visual Studio debugger visualizers (Natvis) for OCCT classes.
   - Added `OSD_Thread::SetName()` property.
- About 117 changes back-ported from OCCT 7.8 and OCCT 7.9 releases.

OCCT collections have been improved to allow C++ range-based loop syntax, e.g.:

```cpp
  for (const TopoDS_Shape& aFaceIter : TopExp_Explorer(theShape, TopAbs_FACE))
  {
    const TopoDS_Face& aFace = TopoDS::Face(aFaceIter);
  }
...
  NCollection_DataMap<int, double> theMap = ...;
  for (const auto& [key, value]: theMap) { ... }
```

Open CASCADE Technology 7.7 (2022-11-03)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.7.0** (2022-11-03) provides about 250 improvements and corrections over the previous release 7.6.0.

- **General**
   - Improved compatibility with `C++17`/`C++20` compilers.
   - Dropped support of pre-`C++11` compilers.
- **Modeling**
   - New functionality is implemented, which could verify the input shape
     to be placed on a canonical geometry with the given tolerance.
     If the input shape is a face or a shell, it could be verified to be close enough
     to Plane, Cylinder, Cone or Sphere.
     If the input shape is an edge or a wire, it could be verified to be close
     to Line, Circle or Ellipse as well as lying on one of the analytical surfaces above.
   - Introduced new tool `BRepLib_PointCloudShape` generating a point set for a topological shape.
   - New option in `BRepOffsetAPI_MakeOffset` - approximation of input contours
     by ones consisting of 2D circular arcs and 2D linear segments only,
     it provides more stable work of 2D offset algorithm.
- **Visualization**
   - Introduced new interface for creating `V3d_View` as subviews of another `V3d_View`.
   - Added smoothing to row interlaced stereoscopic output.
   - Added word-wrapping option to `Font_TextFormatter`.
   - Added support of a wide color window buffer format (10bit per component / 30bit RGB).
   - Added `MSAA` anti-aliasing support when using `WebGL 2.0`.
   - Introduced skydome generation feature `V3d_View::BackgroundSkydome()`.
- **Mesh**
   - `BRepMesh` works too long and produces many free nodes on a valid face problems are resolved.
   - Meshing the shape no longer takes too long and visualization problems are corrected.
   - Wrong shading display of thrusections is fixed.
   - Rendering issue when using deviation coefficient of low value is resolved.
   - Mesher no longer produce 'bad' result for extruded spline with given deviation coefficient.
   - Holes in triangulation with large linear deflection are removed.
   - Broken triangulation on pipe shape is fixed.
- **Data Exchange**
   - `STEP` translator now supports tessellated presentations.
   - Transformation tools `BRepBuilderAPI_Transform`/`BRepBuilderAPI_Copy`
     now handle properly tessellated presentations.
   - `glTF` Writer - added support of `Draco compression`.
   - Introduced `DEWrapper` - a unified interface to Data Exchange connectors.
   - Introduced tool `XCAFDoc_Editor::RescaleGeometry()` for scaling geometry in `XCAF` document.
- **Configuration**
   - `SONAME` is now configurable in `CMake` and includes minor version in addition to major by default.
- **Documentation**
   - Improved samples / tutorials documentation.
   - Introduced new "AIS: Custom Presentation" tutorial.

Open CASCADE Technology 7.6 (2021-11-05)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.6.1**, **7.6.2**, **7.6.3** - bug-fix releases.

**OCCT 7.6.0** (2021-11-05) provides more than 410 improvements and corrections over the previous release 7.5.0.

- **General**
   - Dropped support of `Visual Studio 2008` (finally).
   - Compilation on `ARM64` (`Apple M1`).
   - New interface `Standard::StackTrace()` for dumping stack trace.
   - Removed a set of deprecated classes (`TCollection`, legacy Boolean operation API and others).
- **Modeling**
   - Numerous bug fixes and improved robustness of modeling algorithms.
   - Added progress indication and user break support for
     Boolean operations, `GeomPlate`, `DistShapeShape`, Shape Offset.
   - Added multi-thread mode of `BRepCheck_Analyzer`.
   - Prohibition of scaled transformation within shape location.
- **Visualization**
   - Improved compatibility with `WebGL` and `OpenGL ES`.
   - New interactive object `AIS_LightSource` for a light source.
   - Support for shadow casting using shadow maps (without ray-tracing).
   - Improved performance of rectangular selection.
   - New selection scheme interface to `AIS_InteractiveContext`.
   - New accurate order-independent transparency (OIT) option - depth peeling.
   - `OpenGL` and `OpenGL ES` are no more mutually exclusive graphic drivers
     (`TKOpenGl` and `TKOpenGles` can be now built simultaneously).
   - Ray-Tracing engine is now available within `OpenGL ES 3.2`.
   - Regression testing of `OpenGL ES` graphic driver.
   - More flexible configuration (`Xlib`, `FreeType` can be now excluded).
- **Mesh**
   - Store deflection calculated upon triangulation along with parameters
     passed to a mesher to `Poly_Triangulation`.
- **Data Exchange**
   - Kinematics entities can be read now from a `STEP` file.
   - `glTF` import/export improvements and fixes for passing the validator.
   - Support of `KHR_draco_mesh_compression` extension within `glTF` import.
   - Introduced `OSD_FileSystem` for working with file streams.
- **Application Framework**
   - Improvements of `XCAF` document persistence
     (normal storage, length unit information, option to store in older format).
   - Safe reading of independent `OCAF` documents in different threads.
   - Partial loading of `OCAF` document and appending parts to document.
   - Speed up methods of getting label by entry and vice versa.
- **IVtk**
   - Extraction of per-vertex surface normals for smooth shading.
   - `VTK9` compatibility fixes.
- **Draw Test Harness**
   - `Tk` is now an optional dependency (`USE_TK` in CMake).
   - Support building `DRAWEXE` with statically linked plugins and as `WebAssembly`.
   - Support multi-touch viewer gestures on Windows platform
- **Samples**
   - Fixes for `iOS sample` building.
- **Documentation**
   - Added changelog for `XCafBin` and `XCafXml` storage formats.
   - Added highlighting for code snippets throughout the documentation.

Open CASCADE Technology 7.5 (2020-11-04)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.5.1**, **7.5.2**, **7.5.3** - bug-fix releases.

**OCCT 7.5.0** (2020-11-04) provides about 450 improvements and corrections over the previous release 7.4.0.

- **General**
   - Redesigned API of progress indicator for parallel tasks.
   - Support of compilation for `WebAssembly` (with `Emscripten SDK`).
   - New printer `Message_PrinterSystemLog` for logging messages to system log.
- **Modeling**
   - Support of progress indicator in `BRepMesh`.
   - New alternative algorithm for triangulation of 2d polygons.
   - Tool to remove internal sub-shapes (with `INTERNAL` orientation)
     from the shape keeping topological connectivity.
   - Allowed usage of multi-dimensional compound arguments for Boolean Cut and Common operations.
- **Visualization**
   - Use of `sRGB` textures and render buffer.
   - `PBR Metallic-roughness` shading model.
   - Normal map texture support.
   - Option to compute `BVH` trees used for interactive selection in background thread.
   - Support of non-standard style font families ("Narrow", "Condensed", etc.)
     and multi-font `.ttc` files in Font Manager.
- **Data Exchange**
   - Support of reading `STEP` files containing non-Ascii symbols (`Unicode` or local code pages) in text strings.
   - Support of writing `Unicode` text strings to `STEP` (as `UTF-8`).
   - New API for `STEP` reader accepting `C++ stream` on input.
   - `glTF 2.0` writer.
   - Improved performance of (`ASCII`) `STL` and `OBJ` readers.
- **Application Framework**
   - Management of several documents (open, save, close, etc.) in parallel threads (one application per thread).
   - Inheritance of attributes to reuse their persistence mechanisms.
   - Progress indicator in `TDocStd_Application`.
   - Optimization of Commit operation for large modifications.
- **Draw Test Harness**
   - Colorized message output.
   - Support of Unicode symbols in `DRAW` console on `Windows`.
   - Flight-mode navigation in 3D viewer via `WASD` keys and 3D mouse input on `Windows`.
   - Experimental teleport-mode navigation in 3D viewer using `OpenVR`.
- **Samples**
   - Unification of mouse gestures for 3D viewer manipulation in samples.
   - New WebGL viewer sample.
   - Update of `JNI sample` for `Android Studio` (from `Eclipse` project).
   - New `Qt OCCT Overview` sample.
- **Documentation**
   - Restructuring of OCCT documentation for easier orientation and higher user-friendliness.

Open CASCADE Technology 7.4 (2019-10-01)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.4.0** (2019-10-01) provides more than 500 improvements and corrections over the previous release 7.3.0.

- **Modeling**
   - Improved robustness, performance and accuracy of `BRepMesh` algorithm.
   - Options to control linear and angular deflection for interior part of the faces in.
- **BRepMesh**
   - Improved robustness and stability of Boolean operations and Extrema.
   - Enabled Boolean Operations on open solids.
   - Option to suppress history generation to speed up Boolean Operations.
   - Option to simplify the result of Boolean Operation.
   - Possibility to calculate surface and volume properties of shapes with triangulation-only geometry.
   - A new interface for fetching finite part of infinite box in `BRepBndLib`.
   - New "constant throat" modes of chamfer creation.
   - Removal of API for old Boolean Operations.
- **Visualization**
   - Improved support of `embedded Linux` platforms.
   - Selection performance improvement.
   - Support of clipping planes combinations (clip by box, 3/4, etc.).
   - New class `AIS_ViewController` converting user input (mouse, touchscreen) to camera manipulations.
   - Improved font management .
   - Improved tools for visualization performance analysis.
   - Option to display the outline of shaded objects.
   - Option to exclude seam edges from Wireframe display.
   - Option to display shrunk mesh presentation.
   - Possibility to show shapes with dynamic textures (video).
   - Support of reading encoded bitmap image from memory buffer.
   - Removal of the deprecated Local Context functionality from `AIS`.
   - Removed dependency from `gl2ps` (relying on deprecated `OpenGL` functionality).
- **Data Exchange**
   - New tools to import mesh data from `glTF 2.0` and `OBJ` formats.
   - Support of some non-ASCII encodings in STEP import.
   - Support of `XDE` data (assembly structure, colors, names) in export to `VRML`.
- **Draw Test Harness**
   - Improved 3D Viewer camera manipulations.
   - Fixed issues with starting Draw Harness from batch scripts.
   - Improved support of running Draw Harness in environment without `CASROOT`.
- **Other**
   - Improved performance of built-in parallelization routines (`OSD_Parallel`).
   - Tools for convenient and efficient traverse of `BVH` structures.
   - Optimization of `TPrsStd_AISPresentation` attribute.
   - Sample of 3D Viewer integration in `glfw` application.

Open CASCADE Technology 7.3 (2018-05-29)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.3.0** (2018-05-29) is a minor release with about 350 new features,
improvements and bug fixes over minor release 7.2.0.

- **General**
   - Support of Unicode file names and software signals handling for MinGW-w64 builds.
   - Restored compatibility with `Visual Studio 2008`.
   - Support of `GBK` and `Big5` code pages.
- **Application Framework**
   - Ability to redefine the stored/retrieved document version and the application name.
- **Modeling**
   - Face Removal algorithm.
   - Optimized surface intersection, shape offset and Boolean operation algorithms.
   - Oriented Bounding Boxes (OBB).
   - More complete history in the family of Boolean Operations algorithms.
   - Improved stability of `BRepProj_Projection` algorithm.
- **Visualization**
   - Corrected translation of single-stroke fonts intoBRep.
   - Improved compatibility with `EGL` on `Linux`, Intel HD GPUs, `Mesa OpenGL`, remote desktop.
   - Possibility to arrange more than 8 light sources and assign them to layers.
   - Possibility to assign Shading Model per primitive array.
   - Support of custom `GLSL` programs with Geometry and Tessellation shaders.
   - Distance and size culling options for rendering large scenes.
   - Depth pre-pass option for rendering heavy custom `GLSL` programs.
   - Verbose frame statistics for profiling 3D Viewer performance.
- **Data Exchange **
   - Documentation for `PMI` in `XCAF`.

Open CASCADE Technology 7.2 (2017-08-31)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.2.0** (2017-08-31) is a minor release with more than 500 new features,
improvements and bug fixes over minor release 7.1.0.

- **Configuration**
   - Support of `Visual Studio 2017`.
   - `CMake` option to accelerate build by use of precompiled headers.
- **Application Framework**
   - Dedicated attribute for storage of triangulations.
   - Possibility to save `OCAF` document in `XML` format compatible with OCCT 6.7+.
   - Restored possibility to write shapes in legacy persistence format (`CSFDB`, `ShapeSchema`).
   - Support of files greater than `2 GiB` in binary persistence.
- **Modeling**
   - Optimization of surface intersection and other algorithms.
   - Specialized offset algorithm for smooth shells.
   - Proper setting of regularity on edges connecting smooth surfaces (e.g. seam edges).
   - New algorithm `BOPAlgo_Splitter` allowing to split shapes by intersection with others.
   - New option "Glue" in the family of Boolean algorithms.
   - New error/warning reporting system in Boolean Operations component.
- **Visualization**
   - Order-independent transparency within rasterization rendering.
   - Extended features of color scale presentation.
   - Possibility to customize display of hatching and selection highlight.
   - Multiple improvements in Path Tracing engine.
   - Option for efficient display on high-density screens with low-end graphic cards.
- **Data Exchange**
   - Support of annotations, saved views and clipping planes in `XDE` and `STEP`.
   - Optimized update of assemblies in `XDE`.
   - Support of `PMI` data without semantics in STEP import and export.
   - Support of transparency as part of color specification in `XDE`.
   - Refactored and optimized `STL` read / write module.
- **Test system**
   - Possibility to add custom counters.
   - Interface to connect `DRAW` interpreter to user applications.
- **Samples**
   - New sample for usage of 3D Viewer on `iOS`.

Open CASCADE Technology 7.1 (2016-11-25)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.1.0** (2016-11-25) is a minor release with more than 500 new features,
improvements and bug fixes over major release 7.0.0.

- **General**
   - Support of Windows Store applications.
   - Definition of `Standard_Boolean` as `bool` instead of `unsigned int`.
   - Execution without need of setting environment variables.
- **Application framework**
   - `OCAF` persistence without dynamic plugins.
   - Support of several attributes of one type at the same label.
- **Modeling algorithms**
   - Improved 3D Offset operation in mode Complete with Joint type Intersection.
   - Calculation of the optimal axis-aligned bounding box for a shape.
   - Improved reliability of `HLR` algorithm.
   - Prevention of modification of original shapes in Modeling algorithms.
- **Visualization**
   - Use of programmable pipeline (`GLSL`) by default on all platforms.
   - Improved rendering performance of Wireframe presentation.
   - Improved Shaded highlighting in `AIS_Shape`.
   - Improved clipping planes - capping preserving object material and better performance.
   - Support of perspective projection and selection for transformation persistent objects.
   - New presentation `AIS_Manipulator` for interactive object transformation.
   - New property in `AIS_Dimension` to set custom text value.
   - Type of angle and type of arrows properties in `AIS_AngleDimension`.
   - New optimized Path Tracing algorithm (adaptive screen space sampling).
- **Data exchange**
   - Improved support of `STEP AP242`, including `PMI` data, dimensions, and annotations.

Open CASCADE Technology 7.0 (2016-04-05)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 7.0.0** (2016-04-05) is a major release with more than 500 new features,
improvements and bug fixes over maintenance release 6.9.1.

- **Configuration**
   - Conversion of `CDL` classes to plain C++.
   - Use of `CMake` as main build system, replacing `WOK`.
- **Foundation Classes**
   - New implementation of shared pointer (`Handle`).
   - Redesign of OCCT type system.
- **Modeling algorithms**
   - Refactoring of B-Spline evaluation.
   - Ability to perform Boolean expressions on an arbitrary number of arguments.
   - Intersection of surfaces produces more accurate b-spline curves.
   - More predictable offset of 3D shapes with a large offset value in "Intersection" mode.
- **Visualization**
   - Activation of selection modes without opening the local context.
   - `OpenGL` graphic rendering methods exposed to the client code.
   - Support of zoom persistent selection.
   - Configurable font orientation.
   - `Direct3D` integration toolkit.
   - Support of anti-aliasing using multi-sampling technique (`MSAA`).
- **Application framework**
   - Interface for reading / saving documents from / to arbitrary C++ stream.
   - Main `OCAF` toolkits are made independent on Visualization toolkits.
- **Data exchange**
   - Support of reading and writing semantic `PMI` entities for `STEP AP242` format.
   - Optimization of shape triangulation export to `STL` and `VRML`.

Open CASCADE Technology 6.9 (2015-05-12)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 6.9.1** (2015-09-28) is a maintenance release with more than 200 improvements
and bug fixes over the previous release 6.9.0.
- Highlights
   - Fast shape self-intersection detector.
   - Improved stability and performance of extrema and intersection algorithms.
   - Offset algorithms improved to handle complex geometries.
   - Support of stereoscopic formats recognized by consumer display devices.
   - Possibility to fit 3d viewer to an arbitrary object rather than the whole scene.
   - Extended tools to remove small wires and narrow tails on faces in Shape Healing.
   - History of Boolean operations is accessible in DRAW.

**OCCT 6.9.0** (2015-05-12) is a minor release with about 400 improvements
and bug fixes over the previous release 6.8.0.
- **Modeling algorithms**
   - Fuzzy Boolean operations - possibility to specify global tolerance for a Boolean operation.
   - Support of multiple arguments for a single Boolean operation.
   - Improved performance and thread-safety.
   - Shape proximity detector.
   - Revised usage of `Closed` flag in shapes.
   - Precise evaluation of edge tolerance.
   - Additional options to tune `BRepMesh` algorithm.
   - More robust algorithms of surface-surface intersection and curve on surface projection.
   - Improved support of user feedback messages in Shape Processing.
   - New tool to eliminate small solids in `ShapeFix`.
- **Visualization**
   - Redesign of selection mechanism for better performance.
   - `OpenGL ES 2.0` compatibility improvements.
   - Support of `OpenGL` viewer on `iOS` and ray tracing on `OS X`.
   - Robust implementation of immediate mode using `FBO`.
   - Option to disable automatic re-triangulation of shapes on display.
- **Data Exchange**
   - Reconstruction of p-curves optimized to accelerate import from `STEP`.
   - Colors of edges and faces are written to `IGES 5.3`.
   - Orientation of faces is preserved on export to `IGES 5.1`.
   - Export to `STL` and `VRML 2.0` corrected.
- **Other**
   - Support of parallelism without `TBB` library.
   - `64-bit` mode becomes default on `Windows`.
   - Improved stability of performance measurements in tests.

Open CASCADE Technology 6.8 (2014-11-10)
-----------------------------------------------------------------------------------------------------------------------
**OCCT 6.8.0** (2014-11-10) is a minor release with nearly 600 improvements
and bug fixes over the previous release 6.7.1.

- **Foundation Classes**
   - Using `NULL` as invalid pointer in `Handle` classes, instead of custom constant (`0xfefd0000`...).
   - `STL`-compatible iterators for classes in `NCollection` package.
   - Code clean-up: removing usage of `config.h`, support of pre-standard `STL` streams,
     unused `CDL` template ("generic") classes and instantiations, etc.
- **Modeling**
   - Data structures for Bounded Volume Hierarchy (`BVH`) algorithms.
   - Two new algorithms of global optimization in math, used for Extrema.
   - Parallelization of Boolean Operations algorithm.
   - Interface to break execution of Boolean Operations algorithm.
   - Possibility to protect shape against modification of geometry.
   - Refactoring and optimization of `BRepMesh` algorithm.
   - New option in 2d offset algorithm allowing to keep sharp corners and build one-side offset on open wire.
- **Visualization**
   - New component, `VIS`, providing interactive services (similar to `AIS`) for OCCT shapes in `VTK` viewer.
   - New approach for manipulations with views using camera paradigm.
   - Support of stereoscopic display (requires graphic card supporting `OpenGL Quad Buffer`).
   - Improved support of perspective views.
   - Ray tracing now uses shaders (`GLSL`) instead of `OpenCL`.
   - Possibility to combine objects drawn by `OpenGL` and ray tracing in one view.
   - Frustum culling for fast display of large number of objects with high zoom.
   - Initial support for `OpenGL ES 2.0` for mobile platforms.
   - New classes for display of colored shapes and point clouds in `AIS`.
   - Revised and completed implementation of connected interactive objects.
   - Improved handling of temporary objects in the viewer (immediate mode).
- **Data Exchange**
   - Support of `COMPSOLID`s in `STEP` export.
   - Support of `UNICODE` (`UTF-8`) filenames.
   - Support of names and attributes assigned to points (vertices) in `XCAF`.
- **DRAW**
   - `FPE` signal handlers are disabled by default.
   - New sample scripts demonstrating modeling and visualization capabilities in `DRAW`.
   - Improved usability of top level menu (help browser, samples, User Guide).
- **Documentation**
   - New User Guide describing Boolean Operations algorithms.
   - Improvement of extraction of class documentation for Reference Manual.
- **Samples**
   - New sample for using drawing OCCT 3D viewer into `Direct3D` surface in `WPF` applications.
- **Build system**
   - Support of building on `Android` (except for `DRAW`).

Open CASCADE Technology 6.7 (2013-12-18)
-----------------------------------------------------------------------------------------------------------------------
- **OCCT 6.7.1** (2014-04-30)
   - Maintenance release with about 170 improvements and bug fixes over 6.7.0.
   - Numerous bug fixes and improvements in Modeling Algorithms, Visualization, Data Exchange.
   - Parallelization of the Building and (partially) Intersection parts of Boolean algorithms.
   - Class for display of a shape with different colors of sub-shapes.
   - Extended control over depth buffer operation at the level of Z layers in 3D Viewer.
   - Additional tools and documentation on debugging OCCT code.
   - Support of `SVG` images in documentation.
   - Generation of Reference Manual documentation by `gendoc` command (without `WOK`).
   - Porting of Samples to `Qt5`.
   - `CMake` builds now support source patches and installation of multiple configurations in the same directory.
- **OCCT 6.7.0** (2013-12-18)
   - Minor release with over 350 new features, improvements and bug fixes over 6.6.0.
   - License has been changed to `LGPL-2.1` with additional exception.
   - Dynamic clipping of shaded objects by arbitrary planes, with cross-section visualized by capping algorithm.
   - Optimized and enhanced presentation of point markers using point sprites and `VBO`.
   - Enhanced presentation of dimensions.
   - Support of `GLSL` programs.
   - Ray tracing mode of display of shaded objects in OCCT viewer
     providing high-quality image (including sharp shadows, correct transparency, reflections).
   - Functionality to create BRep shape representing a text string (with specified font).
   - Multiple corrections in `Voxel` package.
   - New (more robust) 2D fillet algorithm.
   - New documentation system: `Doxygen` is used to generate Overview
     and User Guides, their sources are included in OCCT sources.
   - Considerably improved code stability, and clean-up of compiler warnings.
   - Algorithm of result assembling in Boolean Operations is made parallel.
   - Optimization: use of `SSE2` instructions, refactored code of b-spline evaluation,
     revised `XML` persistence of binary data.
   - Refactored `CSharp` sample, now including `WPF` front-end.
   - Support of Intel compiler, `Visual Studio 2013`, and `Mac OS X 10.9 Mavericks`.

Open CASCADE Technology 6.6 (2013-04-22)
-----------------------------------------------------------------------------------------------------------------------
- **OCCT 6.6.0** (2013-04-22)
   - Minor release with over 250 new features, improvements and bug fixes over 6.5.5.
   - Official support of `Mac OS X`, `Windows 8` and `Visual Studio 2012`.
   - On `Mac OS X`, visualization with native `Cocoa` API and `XCode 4` project files.
   - Refactored Boolean operations algorithm.
     Possibility to enable automatic check of input parameters and results and generation of data for bug report.
   - Redesign of texture management.
   - Accelerated text visualization; use of `FTGL` library is dropped.
   - Removal of obsolete 2D viewer and plotter support libraries.
     3D viewer libraries become the unified way to render both 2D and 3D graphics.
   - `TKOpenGl` is now linked at build time, not at run time.
   - Import / export made independent on current global locale.
   - Universal `CMake` build scripts.
   - New automated testing system.

Open CASCADE Technology 6.5 (2011-04-04)
-----------------------------------------------------------------------------------------------------------------------
- **OCCT 6.5.5** (2013-03-29)
   - Maintenance release with 30 improvements and bug fixes over 6.5.4.
   - BREP Format Description White Paper has been added.
- **OCCT 6.5.4** (2012-11-13)
   - Maintenance release with more than 200 improvements and bug fixes over 6.5.3.
   - General code clean-up against compiler warnings, memory issues, and potential errors.
   - Thread safety of B-Spline cache and `BRepMesh` triangulator.
   - Multiple bug fixes in modeling algorithms (intersections, Boolean operations).
   - Improvements of naming mechanism (`OCAF`).
   - Improved stability of Delaunay triangulation algorithm (`BRepMesh`).
   - Improved Visualization (`OpenGL` operations, image processing, display of shading with edges).
   - `FTGL version 2.1.3rc5` supported.
- **OCCT 6.5.3** (2012-04-24)
   - Maintenance release with more than 200 new features, improvements and bug fixes over 6.5.2.
   - Optimization of packages related to interactive selection in 3D view.
   - Redesign of `TKOpenGl` graphic driver.
   - Improved management of gradient and textured background in 3D view.
   - Display of objects in overlay groups.
- **OCCT 6.5.2** (2011-12-12)
   - Maintenance release with 92 new features, improvements and bug fixes over 6.5.1.
   - Redesign of Print operation new Tile algorithm.
   - Support of 2-byte character strings in visualization.
   - Improvement of `BRepMesh` triangulator and Intersection algorithms.
- **OCCT 6.5.1** (2011-06-16)
   - Maintenance release with 57 new features, improvements and bug fixes, over 6.5.0.
   - New tools for memory usage analysis and optimization.
   - Improved performance and correctness of BRepMesh algorithm.
   - Optimization of point-on-surface projection in Extrema package.
   - Multiple improvements to increase robustness.
- **OCCT 6.5.0** (2011-04-04)
   - `FreeImage` library is now used for reading/saving image files.
   - Improved offscreen 3D viewer image dump functionality, which now relies on `OpenGL FBO`.
   - Reading and writing non-manifold topology via `STEP` interface.
   - Trihedron with graduated axes in OCC viewer.

Open CASCADE Technology 6.4 (2010-09-30)
-----------------------------------------------------------------------------------------------------------------------
- **OCCT 6.4.0**
   - Accelerated triangulation algorithms in `BRepMesh` package,
     due to using `TBB` library for the paralleling tools and the memory manager.
   - Improvement of algorithms for line-line, line-plane and plane-plane intersection.
   - New quaternion class for definition and manipulation of 3D rotation operators.
   - Improved rendering performance by using Vertex buffer object (`VBO`).
     `TKOpenGl` implementation converted from C to C++ classes.
   - Use of `freetype` and `ftgl` libraries for advanced 2D and 3D text visualization.

Open CASCADE Technology 6.3 (2008-09-03)
-----------------------------------------------------------------------------------------------------------------------
- **OCCT 6.3.1** (2009-06-19)
   - Maintenance release.
- **OCCT 6.3.0** (2008-09-03)
   - Support of `UTF8` encoding for extended strings, and `Unicode` symbols in `IGES`.
   - Next step in thread-safety: protection against concurrent construction / destruction of `Handle` objects.
   - Improved compatibility with `STL` and `Windows`-specific code.
   - Multiple new features introduced in visualization module.
   - New visualization library `NIS` (New Interactive Service).
   - New standard attributes and numerous improvements in `OCAF`.
   - Changes for `MacOS X` and `FreeBSD` porting.
   - Improved support of perspective view in 3D viewer.
   - New version of the `OCAF` binary persistence format.
   - The functionality of reading/writing `VRML2.0` files has been implemented.
   - The definitions of `Standard_CString` and `Standard_ExtString` (`typedef`s) have been changed to be `const`:
     from `char*` (or `short*`) to `const char*` (or `const short*`).
   - New supported platforms: `Windows Vista`, `Mandriva2006`, `2007`, `2008`, `Debian Etch`, `Red Hat Enterprise 4.0`.
   - New supported compiler: `gcc 4.0`-`4.2`, `Visual C++ 8.0`.

Legacy
-----------------------------------------------------------------------------------------------------------------------
- Open CASCADE Technology 6.2 (2007)
- Open CASCADE Technology 6.1 (2006)
- Open CASCADE Technology 5.2 (2005)
- Open CASCADE Technology 5.1 (2004)
- Open CASCADE 4.0 (2001)
- Open CASCADE 3.0 (2000)
- Open CASCADE 2.0
- Open CASCADE 1.0 (1999-12-07) - first publication under Open CASCADE Technology Public License.
