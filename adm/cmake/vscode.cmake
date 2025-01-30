# VSCode configuration

if(NOT DEFINED VSCODE_WORKSPACE_DIR)
    set(VSCODE_WORKSPACE_DIR ${CMAKE_BINARY_DIR})
endif()

# Put a VSCode workspace file at the top of the build tree
set(VSCODE_WORKSPACE_FILE ${VSCODE_WORKSPACE_DIR}/occt-${CMAKE_BUILD_TYPE}.code-workspace)

include(ProcessorCount)
ProcessorCount(VSCODE_PROC_COUNT)

if(NOT VSCODE_PROC_COUNT)
    set(VSCODE_PROC_COUNT 6)
endif()

if(NOT EXISTS ${VSCODE_WORKSPACE_FILE})
    configure_file(${CMAKE_SOURCE_DIR}/adm/templates/occt.code-workspace.in ${VSCODE_WORKSPACE_FILE})
endif()

# Put launch.json inside .vscode subdir to specify the excutable and environment for debugging
set(VSCODE_CONFIG_DIR ${VSCODE_WORKSPACE_DIR}/.vscode)

if(NOT EXISTS ${VSCODE_CONFIG_DIR})
    file(MAKE_DIRECTORY ${VSCODE_CONFIG_DIR})
endif()

set(VSCODE_LAUNCH_FILE ${VSCODE_CONFIG_DIR}/launch.json)

if(NOT EXISTS ${VSCODE_LAUNCH_FILE})
    if(WIN32)
        set(VSCODE_LAUNCH_EXECUTABLE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DRAWEXE${CMAKE_EXECUTABLE_SUFFIX})
        set(VSCODE_LAUNCH_ENV_VAR "PATH")
        set(VSCODE_LAUNCH_PATH ${3RDPARTY_DLL_DIRS};\${env:PATH})
        set(VSCODE_LAUNCH_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${VSCODE_LAUNCH_PATH})
    elseif(LINUX)
        set(VSCODE_LAUNCH_EXECUTABLE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DRAWEXE${CMAKE_EXECUTABLE_SUFFIX})
        set(VSCODE_LAUNCH_ENV_VAR "LD_LIBRARY_PATH")
        set(VSCODE_LAUNCH_PATH ${3RDPARTY_DLL_DIRS};\${env:PATH})
        set(VSCODE_LAUNCH_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${VSCODE_LAUNCH_PATH})
    elseif(APPLE)
        set(VSCODE_LAUNCH_EXECUTABLE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/DRAWEXE${CMAKE_EXECUTABLE_SUFFIX})
        set(VSCODE_LAUNCH_ENV_VAR "DYLD_LIBRARY_PATH")
        set(VSCODE_LAUNCH_PATH ${3RDPARTY_DLL_DIRS};\${env:DYLD_LIBRARY_PATH})
        set(VSCODE_LAUNCH_PATH ${CMAKE_LIBRARY_OUTPUT_DIRECTORY};${VSCODE_LAUNCH_PATH})
    endif()

    set(VSCODE_DEBUGGER "")
    get_filename_component(VSCODE_DEBUGGER_PATH ${CMAKE_CXX_COMPILER} DIRECTORY)
    get_filename_component(_COMPILER ${CMAKE_CXX_COMPILER} NAME_WLE)
    get_filename_component(_EXT ${CMAKE_CXX_COMPILER} LAST_EXT)

    if(${_COMPILER} STREQUAL "g++")
        set (VSCODE_MIMODE "gdb")
        set(VSCODE_DEBUGGER ${VSCODE_DEBUGGER_PATH}/gdb${_EXT})
    elseif(${_COMPILER} STREQUAL "clang++")
        set (VSCODE_MIMODE "lldb")
        set(VSCODE_DEBUGGER ${VSCODE_DEBUGGER_PATH}/lldb${_EXT})
    endif()

    configure_file(${CMAKE_SOURCE_DIR}/adm/templates/launch.json.in ${VSCODE_LAUNCH_FILE} @ONLY)
    message(STATUS "VSCode launch.json file generated at ${VSCODE_LAUNCH_FILE}")
endif()
