#pragma once
#include "imgui.h"
#include "logger.hpp"
#include <fstream>
#include <string>
#include <filesystem>
#include <format>
#include <unordered_map>

namespace vsa {
  namespace fs = std::filesystem;
  using Config = std::unordered_map<std::string, std::string>;
  
  const fs::path g_download_directory_name = "downloads"; 
  const fs::path g_config_path = fs::path("config") / "client.yaml";

 
  inline void createDownloadDirectory() {
    if(!fs::exists(g_download_directory_name))
      fs::create_directory(g_download_directory_name);
  }

  inline std::filesystem::path newFilePath(const std::string_view& filename) {
    fs::path filepath = g_download_directory_name / filename;          
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

  inline int FilterSpace(ImGuiInputTextCallbackData* callback) {
    if(std::isspace(callback->EventChar))
      return 1;
    return 0;
  }

  inline void ImguiInputTextCustom(const std::string_view& label, std::string& string, int max_length) {
    if(string.capacity() < max_length)
      string.reserve(max_length);
    if(ImGui::InputText(label.data(), string.data(), string.capacity() + 1, 
      ImGuiInputTextFlags_CallbackCharFilter, FilterSpace)) {
      string.resize(strlen(string.c_str()));
    }
  }
  
  inline const Config& getDefaultConfig() {
    static const Config config = { 
      {"address", "localhost" }, 
      {"port", "50000" },
      {"username", "none" },
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
      line_index++;
      int char_index = 0;
      for(char character : cfg_line) {
        if(std::isspace(character))
          continue;
        cfg_line[char_index] = character;
        char_index++;
      }
      cfg_line.resize(char_index);
      int split_pos = cfg_line.find(':');
      if(split_pos == std::string::npos) {
        LOG_ERROR("Could not load a line " << line_index << " from the config file.");
        continue;
      }
      config.insert_or_assign(cfg_line.substr(0, split_pos), cfg_line.substr(split_pos + 1)); 
    }
    cfg_stream.close();
    return true;
  }
}
