#include <optional>
#include <string>
#include <filesystem>

namespace vsa {

  constexpr const char g_upload_directory_name[] = "uploaded_files";

  inline void createUploadDirectory() {
    if(!std::filesystem::exists(g_upload_directory_name))
      std::filesystem::create_directory(g_upload_directory_name);
  }

  inline std::filesystem::path newFilePath(const std::string_view& filename) {
    std::filesystem::path filepath = g_upload_directory_name;
    filepath /= filename;
    
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
    std::string filenames = std::string();
    for(const auto& file : std::filesystem::directory_iterator(g_upload_directory_name)) {
      if(!file.is_regular_file())
        continue;
      size_t file_size = std::filesystem::file_size(file);
      filenames.append(reinterpret_cast<char*>(&file_size), sizeof(size_t));
      filenames.append(file.path().filename().string());
      filenames.push_back('\0');
    }
    return filenames;
  }

  inline std::optional<std::filesystem::path> getFilePath(const std::string_view& filename) {
    std::filesystem::path filepath = g_upload_directory_name;
    filepath /= filename;

    if(!std::filesystem::exists(filepath)) 
      return std::nullopt;
    return filepath;
  }
}
