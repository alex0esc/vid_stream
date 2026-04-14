#pragma once

#if defined(BUILD_WINDOWS)
  #include "windows/screen_capturer_windows.hpp"
#elif defined(BUILD_APPLE)
  #include "mac/screen_capturer_mac.hpp"
#elif defined(BUILD_LINUX)
  #include "linux/screen_capturer_linux.hpp"
#endif

namespace vsa {

  #if defined(BUILD_WINDOWS)
    using CapturerPlatform = CapturerWindows;  
    using DisplayInfoPlatform = DisplayInfoWindows;
  #elif defined(BUILD_APPLE)
    using CapturerPlatform = CapturerApple;
    using DisplayInfoPlatform = DisplayInfoApple;
  #elif defined(BUILD_LINUX)
    using CapturerPlatform = CapturerLinux;
    using DisplayInfoPlatform = DisplayInfoLinux;
  #endif

}
