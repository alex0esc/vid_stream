#include "imgui.h"
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

  inline std::string getSizeText(size_t bytes) {
    std::string size_text;
    if(bytes < 1024) {
      size_text = "\t" + std::to_string(bytes) + "b";
    } else if(bytes < 1024 * 1024) {
      float size = static_cast<float>(bytes) / 1024;
      size_text = "\t" + std::format("{:.2f}", size) + "kb";
    } else if(bytes < 1024 * 1024 * 1024) {
      float size = static_cast<float>(bytes) / 1024 / 1024;
      size_text = "\t" + std::format("{:.2f}", size) + "mb";
    } else if(bytes < static_cast<size_t>(1024) * 1024 * 1024 * 1024) {
      float size = static_cast<float>(bytes) / 1024 / 1024 / 1024;
      size_text = "\t" + std::format("{:.2f}", size) + "gb";
    }
    return size_text;
  }

  inline std::string getTextEllipsed(const std::string_view& text, float max_length) {
    const static float dot_length = ImGui::CalcTextSize("...").x; 
    max_length -= dot_length;
    size_t new_text_length = 0;
    while (new_text_length < text.length()) {
      if(ImGui::CalcTextSize(text.data(), text.data() + new_text_length + 1).x > max_length) 
        break;
      new_text_length++;
    } 
    std::string new_text = std::string(text.data(), new_text_length);
    if(new_text_length < text.length()) 
      new_text.append("...");
    return new_text;
  }
}
