#include "Wepp/FileHandling/FileIO.hpp"
#include <fstream>

namespace Wepp {

size_t FileSize(const std::filesystem::path &_filename) {
  size_t output;
  std::fstream file;
  file.open(_filename, std::ios::in | std::ios::ate);
  output = file.tellg();
  file.close();
  return output;
}

void ReadFile(const std::filesystem::path &_filename,
              std::vector<unsigned char> &_buffer) {
  size_t size = FileSize(_filename);
  _buffer.resize(size);

  std::fstream file;
  file.open(_filename, std::ios::in | std::ios::binary);
  file.read((char *)_buffer.data(), _buffer.size());
  file.close();
}
} // namespace Wepp
