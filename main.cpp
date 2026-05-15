#include <atomic>
#include <charconv>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <utility>

#include "crow.h"
#include "naive-file-filter.hpp"

void printHelp() {
  const auto* help =
      "Usage: <program> <seconds> [<directory> [http|file [config_file]]]\n"
      "\t<seconds> - update interval in seconds;\n"
      "\t<directory> - directory to traverse, default value is $HOME;\n"
      "\thttp|file - output mode (http or file), default file;\n"
      "\tconfig_file - file with extensions to treat as media;\n"
      "After startup, enter 'exit' to stop the program.\n";
  std::cout << help;
}

void writeResult(std::ostream& output, MediaFiles files) {
  output << "{ ";
  constexpr std::array media_file_types = {"audio", "video", "images"};
  static_assert(media_file_types.size() == files.size());
  for (auto i = 0; i < files.size(); i++) {
    output << std::quoted(media_file_types[i]) << ": [ ";
    for (auto j = 0; j < files[i].size(); j++) {
      output << std::quoted(files[i][j]);
      if (j != files[i].size() - 1) {
        output << ", ";
      }
    }
    output << " ]";
    if (i != files.size() - 1) {
      output << ", ";
    }
  }
  output << " }\n";
}

void writeResultInFile(const std::filesystem::path& directory,
                       MediaFiles files) {
  const char* result_filename = ".media_files";
  std::ofstream result_file(directory / result_filename,
                            std::ios_base::out | std::ios_base::trunc);
  writeResult(result_file, std::move(files));
}

class ResultStorage {
 public:
  static std::string getResultStr() {
    std::lock_guard lock{m_};
    return result_;
  }

  static void updateResultStr(std::string new_result) {
    std::lock_guard lock{m_};
    result_ = std::move(new_result);
  }

 private:
  static inline std::string result_{};
  static inline std::mutex m_{};
};

void writeResultInStorage(MediaFiles files) {
  std::stringstream oss;
  writeResult(oss, std::move(files));
  ResultStorage::updateResultStr(oss.str());
}

void startTraversing(const int seconds, const std::filesystem::path& directory,
                     FileFilter* filter, bool http,
                     std::atomic<bool>& stop_flag) {
  while (!stop_flag.load()) {
    MediaFiles files;
    for (std::filesystem::recursive_directory_iterator iter(
             directory,
             std::filesystem::directory_options::skip_permission_denied);
         const auto& dir_entry : iter) {
      if (dir_entry.is_directory()) {
        continue;
      }
      const auto& entry_path = dir_entry.path();
      auto file_type = filter->determineFileType(entry_path);
      if (file_type == FileType::NON_MEDIA) {
        continue;
      }
      files[static_cast<std::size_t>(file_type)].emplace_back(
          entry_path.string());
    }
    if (http) {
      writeResultInStorage(files);
    } else {
      writeResultInFile(directory, files);
    }

    for (int i = 0; i < seconds && !stop_flag.load(); i++) {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }
}

int main(const int argc, char const* argv[]) {
  if (argc < 2 || argc > 5) {
    printHelp();
    return 0;
  }

  bool http = false;
  int seconds = 0;
  {
    const char* seconds_str = argv[1];
    const char* seconds_str_end = seconds_str + std::strlen(seconds_str);
    const auto result = std::from_chars(seconds_str, seconds_str_end, seconds);
    if (result.ptr != seconds_str_end) {
      std::cerr << "Wrong arguments\n";
      return 2;
    }
  }

  std::filesystem::path directory;
  if (argc < 3) {
    const char* home_path = std::getenv("HOME");
    if (home_path == nullptr) {
      std::cerr << "Unable to read $HOME\n";
      return 1;
    }
    directory = home_path;
  } else {
    directory = argv[2];
  }

  if (argc >= 4) {
    if (std::string_view(argv[3]) == "http") {
      http = true;
      std::jthread http_thread([]() {
        crow::SimpleApp app;
        CROW_ROUTE(app, "/media_files")([]() {
          const std::string response = ResultStorage::getResultStr();
          crow::response res(200, response);
          res.set_header("Content-Type", "application/json");
          return res;
        });
        app.port(1234).run();
      });
      http_thread.detach();
    } else if (std::string_view(argv[3]) == "file") {
      http = false;
    } else {
      std::cerr << "Unrecognized arguments\n";
      return 3;
    }
  }

  NaiveFileFilter file_filter;
  if (argc >= 5) {
    file_filter = NaiveFileFilter(NaiveFileFilter::loadFromConfig(argv[4]));
  }

  std::atomic<bool> stop_flag{false};
  std::jthread worker([&] {
    startTraversing(seconds, directory, &file_filter, http, stop_flag);
  });

  std::string command;
  while (true) {
    std::cin >> command;
    if (command == "exit") {
      stop_flag.store(true);
      break;
    }
  }

  return 0;
}
