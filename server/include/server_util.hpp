#pragma once
#include "logger.hpp"
#include <cctype>
#include <fstream>
#include <optional>
#include <string>
#include <filesystem>
#include <unordered_map>

namespace vsa {
  namespace fs = std::filesystem;
  using Config = std::unordered_map<std::string, std::string>;

  const fs::path g_upload_directory_name = "uploaded_files";
  const fs::path g_config_path = fs::path("config") / "server.yaml";

  inline void createUploadDirectory() {
    if(!fs::exists(g_upload_directory_name))
      fs::create_directory(g_upload_directory_name);
  }

  inline fs::path newFilePath(const std::string_view& filename) {
    fs::path filepath = g_upload_directory_name / filename;
    
    while(fs::exists(filepath)) {
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

  inline std::optional<fs::path> getFilePath(const std::string_view& filename) {
    fs::path filepath = g_upload_directory_name / filename;

    if(!fs::exists(filepath)) 
      return std::nullopt;
    return filepath;
  }

  inline const Config& getDefaultConfig() {
    static const Config config = { 
      {"password", "pw123" } 
    }; 
    return config;
  }

  inline void saveConfig(const Config& config) {
    std::fstream cfg_stream = std::fstream(g_config_path, std::ios::out | std::ios::trunc);
    for(const auto& [key, value] : config) {
      std::string line = key + ": " + value + "\n";
      cfg_stream.write(line.c_str(), line.length());
    }
    cfg_stream.close();
  }

  inline bool loadConfig(Config& config) {
    if(!fs::exists(g_config_path)) 
      return false;
    std::string cfg_line;
    std::fstream cfg_stream = std::fstream(g_config_path, std::ios::in);
    int line_index = 0;
    while(std::getline(cfg_stream, cfg_line, '\n')) {
      int lineindex = 0;
      for(char character : cfg_line) {
        if(std::isspace(character))
          continue;
        cfg_line[lineindex] = character;
        lineindex++;
      }
      cfg_line.resize(lineindex);
      int split_pos = cfg_line.find(':');
      if(split_pos == std::string::npos) {
        LOG_ERROR("Could not load a line " << line_index << " from the config file.");
        continue;
      }
      config.insert_or_assign(cfg_line.substr(0, split_pos), cfg_line.substr(split_pos + 1)); 
      line_index++;
    }
    cfg_stream.close();
    return true;
  }
}
