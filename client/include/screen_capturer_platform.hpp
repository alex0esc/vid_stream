#pragma once

#if defined(_WIN32)
  #include "screen_capturer_windows.hpp"
#endif

namespace vsa {

  #if defined(_WIN32)
    using CapturerPlatform = CapturerWindows;  
  #endif

}
