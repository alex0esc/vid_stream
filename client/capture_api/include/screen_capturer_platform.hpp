#pragma once

#if defined(_WIN32)
  #include "windows/screen_capturer_windows.hpp"
#elif defined(__APPLE__)
  #include "mac/screen_capturer_mac.hpp"
#elif defined(__linux__)
  #include "linux/screen_capturer_linux.hpp"
#endif

namespace vsa {

  #if defined(_WIN32)
    using CapturerPlatform = CapturerWindows;  
  #elif defined(__APPLE__)
    using CapturerPlatform = CapturerMac;
  #elif defined(__linux__)
    using CapturerPlatform = CapturerLinux;
  #endif

}
