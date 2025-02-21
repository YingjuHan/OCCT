Open CASCADE Technology
=======================

## About this project

![Open CASCADE Technology logo](/dox/resources/occt_draw_logo.png)

This project is a fork of the Open CASCADE Technology (OCCT).
Currently the project aims at gathering patches/changes/improvements to the branch `v7_7_x`
preserving API compatibility with OCCT 7.7 as much as possible/reasonable.

The project includes a set of patches back-ported from newer OCCT releases (with some of them amended to avoid regressions),
as well as independently developed bugfixes and new features (see git log for details).

## About Open CASCADE Technology

Open CASCADE Technology (OCCT) is an open source full-scale 3D geometry library.
Striving to be one of the best free cad software kernels, OCCT is widely used for the development
of specialized programs dealing with the following engineering and mechanical domains:
3D modeling (CAD), manufacturing (CAM), numerical simulation (CAE), measurement equipment (CMM) and quality control (CAQ).
Since its publication in 1999 as an open source CAD software kernel,
OCCT has been successfully used in numerous projects ranging from building and construction to aerospace and automotive.

## License

Open CASCADE Technology is free software; you can redistribute it and / or modify it
under the terms of the [GNU Lesser General Public version 2.1](LICENSE_LGPL_21.txt) as published by the Free Software Foundation,
with special exception defined in the file [OCCT_LGPL_EXCEPTION.txt](OCCT_LGPL_EXCEPTION.txt).

See [dox/license.md](dox/license.md) for complete text of the license.

## Build and Install

Use CMake to configure the project. See the [building_occt.md](dox/build/build_occt/building_occt.md) for details.

## Version

The current version of OCCT can be consulted in the file [src/Standard/Standard_Version.hxx](src/Standard/Standard_Version.hxx).

Check [changelog.md](dox/changelog/changelog.md) for highlights of released versions.

Check also [upgrade.md](dox/upgrade/upgrade.md) for porting hints between releases.

## Documentation

The documentation is located within `dox` folder in git repository.
It should be processed by Doxygen to generate HTML with navigation structure, images and C++ references,
see [building_documentation.md](dox/build/build_documentation/building_documentation.md).

The framework consists of the following modules:
- [**Foundation Classes**](dox/user_guides/foundation_classes/foundation_classes.md)
  module underlies all other OCCT classes;
- [**Modeling Data**](dox/user_guides/modeling_data/modeling_data.md)
  module supplies data structures to represent 2D and 3D geometric primitives and their compositions into CAD models;
- [**Modeling Algorithms**](dox/user_guides/modeling_algos/modeling_algos.md)
  module contains a vast range of geometrical and topological algorithms;
  * Including [**TKMesh**](dox/user_guides/mesh/mesh.md) toolkit generating tessellated representations of B-Rep objects;
- [**Visualization**](dox/user_guides/visualization/visualization.md)
  module provides mechanisms for graphical data representation in a 3D Viewer (OpenGL/OpenGL ES/WebGL);
- [**Data Exchange**](dox/user_guides/xde/xde.md) module inter-operates with popular data formats
  and relies on [**Shape Healing**](dox/user_guides/shape_healing/shape_healing.md)
  to improve compatibility between CAD software of different vendors;
  * **STEP** (AP203: Mechanical Design, this covers General 3D CAD; AP214: Automotive Design; AP242);
  * **IGES** (up to 5.3);
  * **glTF** 2.0 reader and writer;
  * **OBJ** mesh file reader and writer;
  * **VRML** converter translates Open CASCADE shapes to VRML 1.0 files (Virtual Reality Modeling Language);
  * **STL** converter translates Open CASCADE shapes to STL files.
- [**Application Framework**](dox/user_guides/ocaf/ocaf.md)
  module offers ready-to-use solutions for handling application-specific data (user attributes)
  and commonly used functionality (save/restore, undo/redo, copy/paste, tracking CAD modifications, etc.);
- [**Draw Harness**](dox/user_guides/draw_test_harness/draw_test_harness.md)
  framework exposing access to other OCCT algorithms through Tcl command interpreter
  for interactive evaluation of algorithms, prototyping, demonstration, scripting
  and automated [regression testing](dox/contribution/tests/tests.md).

See also [introduction.md](dox/introduction/introduction.md).
