# Verify that the two copies of every GLSL resource stay in sync.
#
# OCCT ships each shader twice:
#   - resources/Shaders/<name>.{glsl,fs,vs} - the authoritative source, read
#     at run time through CSF_ShadersDirectory (Graphic3d_ShaderProgram::ShadersFolder());
#   - src/Visualization/TKService/Shaders/Shaders_<name>.pxx - a C++ array
#     generated from that resource by adm/cmake/occt_resources.cmake and
#     #included directly by the sources, used as a fallback when no resource
#     directory is available.
#
# Both copies are live, so editing only one of them is a silent defect: the
# embedded copy still compiles the previous shader. This happened in this
# repository - Shaders_PhongShading_fs.pxx had drifted from PhongShading.fs,
# and the stale copy was picked up whenever CSF_ShadersDirectory was unset.
#
# The check regenerates the expected .pxx content exactly the way
# occt_resources.cmake does and compares it against the committed file.
# It runs at configure time and never modifies anything.
#
# Set BUILD_RESOURCES=ON to regenerate the .pxx files instead of checking them.

function (OCCT_CHECK_SHADER_RESOURCE_SYNC THE_RESOURCE_DIR THE_PXX_DIR)
  set (THE_MISMATCHED)

  file (STRINGS "${OCCT_ROOT_DIR}/${THE_RESOURCE_DIR}/FILES" THE_RESOURCE_FILES)
  foreach (THE_ENTRY ${THE_RESOURCE_FILES})
    string (REGEX MATCH "^[^:]+:::" THE_IS_RESOURCE "${THE_ENTRY}")
    if (NOT THE_IS_RESOURCE)
      continue()
    endif()
    string (REGEX REPLACE "[^:]+:+" "" THE_RESOURCE_FILE "${THE_ENTRY}")

    get_filename_component (THE_BASE_NAME "${THE_RESOURCE_FILE}" NAME)
    string (REPLACE "." "_" THE_BASE_NAME "${THE_BASE_NAME}")
    set (THE_PXX_NAME "Shaders_${THE_BASE_NAME}.pxx")
    set (THE_PXX_PATH "${OCCT_ROOT_DIR}/${THE_PXX_DIR}/${THE_PXX_NAME}")
    set (THE_RESOURCE_PATH "${OCCT_ROOT_DIR}/${THE_RESOURCE_DIR}/${THE_RESOURCE_FILE}")

    if (NOT EXISTS "${THE_PXX_PATH}")
      list (APPEND THE_MISMATCHED "${THE_RESOURCE_FILE}: no generated copy ${THE_PXX_NAME}")
      continue()
    endif()

    # Rebuild the expected payload. IN LISTS is required here: it preserves
    # the empty lines every shader has, while a plain ${VAR} expansion would
    # silently drop them.
    file (STRINGS "${THE_RESOURCE_PATH}" THE_RESOURCE_LINES)
    get_filename_component (THE_SYMBOL_BASE "${THE_RESOURCE_FILE}" NAME)
    string (REPLACE "." "_" THE_SYMBOL_BASE "${THE_SYMBOL_BASE}")
    set (THE_EXPECTED "static const char Shaders_${THE_SYMBOL_BASE}[] =")
    foreach (THE_LINE IN LISTS THE_RESOURCE_LINES)
      string (REPLACE "\"" "\\\"" THE_LINE "${THE_LINE}")
      set (THE_EXPECTED "${THE_EXPECTED}\n  \"${THE_LINE}\\n\"")
    endforeach()
    set (THE_EXPECTED "${THE_EXPECTED};\n")

    # Everything around the array comes from adm/templates/header.in - the
    # license header in front and the "// clang-format on" line after the
    # closing ';' - and is not derived from the resource, so compare only the
    # array itself: from "static const char" up to and including the ';' that
    # terminates it. That ';' is the only one at the end of a line, since the
    # semicolons inside the shader body are always followed by more text.
    file (READ "${THE_PXX_PATH}" THE_ACTUAL)
    string (FIND "${THE_ACTUAL}" "static const char" THE_ACTUAL_POS)
    if (THE_ACTUAL_POS LESS 0)
      list (APPEND THE_MISMATCHED "${THE_PXX_NAME}: no shader array found")
      continue()
    endif()
    string (SUBSTRING "${THE_ACTUAL}" ${THE_ACTUAL_POS} -1 THE_ACTUAL_ARRAY)

    # The last element of the array ends with the two-character sequence \n
    # (an escaped newline inside the C string literal) followed by the closing
    # quote and the ';' that terminates the declaration. In a CMake string
    # that literal backslash has to be written as \\n.
    set (THE_TERMINATOR "\\n\";\n")
    string (FIND "${THE_ACTUAL_ARRAY}" "${THE_TERMINATOR}" THE_CUT_POS)
    string (FIND "${THE_EXPECTED}" "${THE_TERMINATOR}" THE_EXPECTED_CUT_POS)
    if (THE_CUT_POS LESS 0 OR THE_EXPECTED_CUT_POS LESS 0)
      list (APPEND THE_MISMATCHED "${THE_PXX_NAME}: unterminated shader array")
      continue()
    endif()
    math (EXPR THE_CUT_END "${THE_CUT_POS} + 5")
    math (EXPR THE_EXPECTED_CUT_END "${THE_EXPECTED_CUT_POS} + 5")
    string (SUBSTRING "${THE_ACTUAL_ARRAY}" 0 ${THE_CUT_END} THE_ACTUAL_ARRAY)
    string (SUBSTRING "${THE_EXPECTED}" 0 ${THE_EXPECTED_CUT_END} THE_EXPECTED)

    if (NOT THE_ACTUAL_ARRAY STREQUAL THE_EXPECTED)
      list (APPEND THE_MISMATCHED
        "${THE_RESOURCE_DIR}/${THE_RESOURCE_FILE} != ${THE_PXX_NAME} (embedded copy is stale)")
    endif()
  endforeach()

  list (LENGTH THE_MISMATCHED THE_MISMATCHED_NUM)
  if (THE_MISMATCHED_NUM GREATER 0)
    if (BUILD_RESOURCES)
      message (WARNING "Warning: regenerating embedded shader copies out of sync with resources:")
    else()
      message (WARNING "Warning: embedded shader copies are out of sync with resources:")
    endif()
    foreach (THE_ITEM ${THE_MISMATCHED})
      message (WARNING "  ${THE_ITEM}")
    endforeach()
    if (NOT BUILD_RESOURCES)
      message (WARNING "Each GLSL resource is embedded twice: once under ${THE_RESOURCE_DIR} and once as a")
      message (WARNING "C++ array under ${THE_PXX_DIR}. Both copies are used at run time, so they must be")
      message (WARNING "updated together. Re-run CMake with -DBUILD_RESOURCES=ON to regenerate the")
      message (WARNING "embedded copies from the resources, or revert the incomplete edit.")
    endif()
  else()
    message (STATUS "Info: embedded shader copies are in sync with ${THE_RESOURCE_DIR}")
  endif()
endfunction()
