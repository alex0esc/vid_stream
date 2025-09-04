#include <string>
#include <filesystem>

namespace vsa {

 constexpr const char g_download_directory_name[] = "downloads"; 

  inline std::filesystem::path getFilePath(const std::string_view& filename) {
    std::filesystem::path filepath = g_download_directory_name;
    if(!std::filesystem::exists(filepath)) 
        std::filesystem::create_directory(filepath);
    filepath /= filename;          
    return filepath;
  }
  
  inline std::string toLowerCase(const std::string_view& str) {
    std::string lower_case_str = std::string();
    for(char character : str) {
      lower_case_str.push_back(std::tolower(character));
    }
    return lower_case_str;
  }
}
