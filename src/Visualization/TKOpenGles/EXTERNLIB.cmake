# External dependencies for TKOpenGles
set(OCCT_TKOpenGles_EXTERNAL_LIBS
  TKernel
  TKService
  TKMath
  CSF_TBB
  CSF_FREETYPE
  CSF_OpenGlesLibs
  CSF_user32
  CSF_gdi32
  CSF_XwLibs
  CSF_Appkit
  CSF_IOKit
  # macOS: ANGLE 的 EGL window surface 需要绑定到 CAMetalLayer（QuartzCore）。
  # 该项由 occt_csf.cmake 在 USE_GLES2 时设置；为空时会被自动跳过。
  CSF_QuartzCore
  CSF_objc
)
