#ifndef KVADRAOS4_FILE_FILTER_HPP
#define KVADRAOS4_FILE_FILTER_HPP

#include <filesystem>
#include <cstdint>

constexpr auto MediaFileTypes = 3;
enum class FileType : std::uint8_t {
  AUDIO = 0, VIDEO, IMAGE, NON_MEDIA
};
using MediaFiles = std::array<std::vector<std::string>, MediaFileTypes>;

class FileFilter {
 public:

  virtual FileType determineFileType(const std::filesystem::path &path) = 0;

  virtual ~FileFilter() = default;
};

#endif  // KVADRAOS4_FILE_FILTER_HPP
