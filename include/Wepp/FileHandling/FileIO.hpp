#pragma once
#include <cstddef>
#include <filesystem>
#include <vector>

namespace Wepp {

size_t FileSize(const std::filesystem::path &_filename);
void ReadFile(const std::filesystem::path &_filename, std::vector<unsigned char> &_buffer);
} // namespace Wepp
