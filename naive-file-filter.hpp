#ifndef KVADRAOS4_NAIVE_FILE_FILTER_HPP
#define KVADRAOS4_NAIVE_FILE_FILTER_HPP

#include <array>
#include <string>
#include <utility>
#include <vector>

#include "file-filter.hpp"

class NaiveFileFilter : public FileFilter {
 public:
  NaiveFileFilter()
      : extensions_{std::vector<std::string>{"mp3", "wav"},
                   std::vector<std::string>{"mpg", "mp4"},
                   std::vector<std::string>{"jpeg", "png"}} {}

  explicit NaiveFileFilter(MediaFiles extensions) : extensions_(std::move(extensions)) {}

  static MediaFiles loadFromConfig(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
      throw std::runtime_error("Cannot open config file: " + config_path);
    }

    MediaFiles extensions;
    int line_num = 0;

    while (line_num < MediaFileTypes) {
      std::string line;
      std::getline(file, line);

      std::istringstream line_stream(line);
      std::string ext;
      while (true) {
        line_stream >> ext;
        if (!ext.empty()) {
          extensions[line_num].emplace_back(std::move(ext));
        }
        if (!line_stream.good()) {
          break;
        }
      }
      ++line_num;
    }

    return extensions;
  }

  FileType determineFileType(const std::filesystem::path& path) override {
    const auto path_ext_with_dot = path.extension().string();
    if (path_ext_with_dot.empty()) {
      return FileType::NON_MEDIA;
    }
    const auto path_ext = std::string_view(path_ext_with_dot).substr(1);
    for (int ext_type = 0; ext_type < extensions_.size(); ext_type++) {
      for (const auto& ext : extensions_[ext_type]) {
        if (path_ext == ext) {
          return static_cast<FileType>(ext_type);
        }
      }
    }
    return FileType::NON_MEDIA;
  }

  ~NaiveFileFilter() override = default;

 private:
  MediaFiles extensions_{};
};

#endif  // KVADRAOS4_NAIVE_FILE_FILTER_HPP
