#include "logger.hpp"
#include <fstream>
#include <optional>
#include <string>
#include <filesystem>


namespace vsa {

  constexpr const char g_upload_directory_name[] = "uploaded_files";

  inline std::filesystem::path newFilePath(const std::string_view& filename) {
    std::filesystem::path filepath = g_upload_directory_name;
    if(!std::filesystem::exists(filepath)) 
        std::filesystem::create_directory(filepath);
    
    filepath /= std::filesystem::path(filename);
    
    while(std::filesystem::exists(filepath)) {
      size_t last_dot = filepath.filename().string().find_last_of(".");
      if(last_dot == std::string::npos) {
        filepath.replace_filename(filepath.filename().string() + "-cpy");
      } else {
        filepath.replace_filename(filepath.filename().string().substr(0, last_dot) + "-cpy" + filepath.filename().string().substr(last_dot));
      }
    }    
    return filepath;
  } 

  inline std::string getFileList() {
    std::string file_names = std::string();
    std::filesystem::path filepath = g_upload_directory_name;
    for(const auto& file : std::filesystem::directory_iterator(filepath)) {
      if(!file.is_regular_file())
        continue;
      if(file_names.length() != 0)
        file_names.push_back('\0');
      file_names.append(file.path().filename().string());
    }
    return file_names;
  }

  inline std::optional<std::filesystem::path> getFilePath(const std::string_view& filename) {
    std::filesystem::path filepath = g_upload_directory_name;
    filepath /= filename;
    if(!std::filesystem::exists(filepath)) {
      LOG_ERROR("The file " << filename << " does not exist.");
      return std::nullopt;
    }
    return filepath;
  }
}
