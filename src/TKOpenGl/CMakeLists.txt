project(TKOpenGl)

set (TKOpenGl_DISABLE_COTIRE ON)

OCCT_INCLUDE_CMAKE_FILE (adm/cmake/occt_toolkit)

if (USE_WAYLAND)
  target_link_libraries (${PROJECT_NAME} EGL GL)
endif()
