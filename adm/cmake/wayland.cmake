# wayland

include (FindPkgConfig)
if (BUILD_RESOURCES)
  pkg_check_modules (WAYLAND REQUIRED IMPORTED_TARGET
                     wayland-client>=0.2.7 wayland-cursor>=0.2.7 wayland-egl>=0.2.7 wayland-protocols wayland-scanner)
  pkg_get_variable (WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
  pkg_get_variable (WAYLAND_SCANNER wayland-scanner wayland_scanner)
else()
  pkg_check_modules (WAYLAND REQUIRED IMPORTED_TARGET
                     wayland-client>=0.2.7 wayland-cursor>=0.2.7 wayland-egl>=0.2.7)
endif()

# Generates C++ wayland protocols from XML definition using wayland-scanner.
macro (OCCT_GENERATE_WAYLAND_PROTOCOL theOutFolder theProtocol)
  set (aProtXml "${WAYLAND_PROTOCOLS_DATADIR}/stable/${theProtocol}/${theProtocol}.xml")
  set (aProtHpp "${theOutFolder}/${theProtocol}-client-protocol.pxx")
  set (aProtCpp "${theOutFolder}/${theProtocol}-client-protocol-impl.pxx")

  execute_process (COMMAND "${WAYLAND_SCANNER}" client-header "${aProtXml}" "${aProtHpp}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} RESULT_VARIABLE isGenHpp
  )
  if (NOT ${isGenHpp} STREQUAL "0")
    message (FATAL_ERROR "${WAYLAND_SCANNER} is unable to generate ${aProtHpp} from ${aProtXml}")
  endif()
  message (STATUS "${WAYLAND_SCANNER} generated ${aProtHpp} from ${aProtXml}")
  execute_process (COMMAND "${WAYLAND_SCANNER}" private-code  "${aProtXml}" "${aProtCpp}"
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR} RESULT_VARIABLE isGenCpp
  )
  if (NOT ${isGenCpp} STREQUAL "0")
    message (FATAL_ERROR "${WAYLAND_SCANNER} is unable to generate ${aProtCpp} from ${aProtXml}")
  endif()
  message (STATUS "${WAYLAND_SCANNER} generated ${aProtCpp} from ${aProtXml}")
endmacro()
