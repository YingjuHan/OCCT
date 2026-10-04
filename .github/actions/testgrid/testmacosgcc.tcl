# Same as testmacos.tcl, used by the macOS + GCC configuration.
# See the comments in testmacos.tcl for the exclude policy.

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
testgrid -exclude {*}$exclude_str -outdir results/macos-gcc-x64
