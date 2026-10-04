# Test script executed on macOS by .github/workflows/build-and-test-multiplatform.yml
# (job "test-macos-x64", driven by .github/actions/run-tests/action.yml).
#
# Run the WHOLE test grid, the same way testwindows.tcl and testlinuxgcc.tcl do.
# Before this change the file contained a single line -
#   testgrid -outdir results/macos-x64 caf basic
# which meant that 39 of 40 test groups never ran on macOS, so platform
# regressions stayed invisible to CI.
#
# Exclude policy (identical to the other platforms):
#   - list only what cannot pass on this platform, or what is already excluded
#     on another platform for the same reason;
#   - an entry without a valid reason is a hidden regression - remove it.
#
# IMPORTANT: the entries below are architecture-independent (OpenGL / ANGLE
# capabilities, shared test-script defects). They were collected on Apple arm64.
# The CI runner is macos-15 (Intel x64): a number of cases that fail locally
# because of arm64 vs x86-64 floating point differences are deliberately NOT
# listed here, because they may well pass on CI. Triage the first CI run before
# adding anything else.
#
# Note: comments cannot be placed inside the [list ...] construct below - Tcl
# would treat "#" as a regular word there and the command would break.
#
# Known failures NOT listed here (they are real defects to fix, not platform
# limitations, so they must stay visible - do not hide them by adding entries):
#   * arm64 vs x86-64 numeric differences in modelling / meshing algorithms
#     (bugs modalg_5 bug23706_*, bugs mesh bug*, v3d bugs bug288_5 ...);
#     these may pass on the Intel CI runner, so measure before excluding;
#   * a teardown crash "OSD_SIGSEGV ... Address 97" at DRAW exit
#     (v3d bugs bug23670_* and friends, opengl/*) - root cause identified:
#     OpenGl_ShaderManager::myContext is already null while
#     V3d_View::Redraw() is still called from ViewerTest_ExitProc;
#   * "FBO blitting has failed [GL_INVALID_OPERATION]" in multiview cases
#     (opengl general multiview1/multiview6) - OCCT-side FBO state handling.

# "opengles3 raytrace" (whole grid): OpenGL ES on macOS is provided by ANGLE,
#   which caps at GLES 3.0. OCCT ray tracing requires ES >= 3.2 and
#   GL_EXT_texture_buffer (absent), so 21 of the 22 cases cannot pass at all.
# "opengl raytrace ...": desktop OpenGL on macOS is capped at 4.1 by Apple,
#   while these cases need 4.2+ features; they fail with
#   "Unhandled GL error: GL_INVALID_OPERATION" while setting up FBOs.
# "perf mesh ...": same FBO error; measured timings are meaningless while
#   rendering errors out.
# "bugs modalg_5 bug25199", "lowalgos intss bug567_1": already excluded on Linux
#   for numeric differences in intersection / modelling algorithms.
# "bugs modalg_7 bug83", "opengl background srgb": already excluded on Windows.
set exclude_list [list \
    "opengles3 raytrace" \
    "opengl raytrace bug24130" \
    "opengl raytrace bug24819" \
    "opengl raytrace bug26617" \
    "opengl raytrace bug27083" \
    "opengl raytrace bug31279" \
    "opengl raytrace sample_ball" \
    "perf mesh bug25264" \
    "perf mesh bug26889_1" \
    "perf mesh bug26889_2" \
    "perf mesh bug26889_3" \
    "perf mesh bug26965" \
    "bugs modalg_5 bug25199" \
    "lowalgos intss bug567_1" \
    "bugs modalg_7 bug83" \
    "opengl background srgb" \
]

set exclude_str [join $exclude_list ,]
testgrid -exclude {*}$exclude_str -outdir results/macos-x64
