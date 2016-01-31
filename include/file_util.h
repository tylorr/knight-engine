#pragma once

#include "gsl.h"
#include "types.h"
#include "vector.h"
#include "string_stream.h"

#include <collection_types.h>
#include <gsl.h>

#include <cstdint>
#include <fstream>
#include <tuple>

namespace knight {

struct DirectoryContent {
  DirectoryContent(foundation::Allocator &alloc) :
    path{alloc} { }

  string_stream::Buffer path;

  int depth;
};

namespace file_util {

uint64_t GetLastWriteTime(gsl::czstring<> filename);

bool ListDirectoryContents(foundation::Allocator &allocator, gsl::czstring<> directory_path);

template<typename CharT = char>
std::tuple<Vector<CharT>, bool> read_file_to_buffer(foundation::Allocator &allocator, gsl::czstring<> file_path) {
  Vector<CharT> buffer{allocator};
  std::basic_ifstream<CharT> file{file_path, std::ios::binary};
  if (file.fail()) {
    return std::make_tuple(buffer, false);
  }

  file.seekg(0, std::ios::end);

  auto file_size = int64_t{file.tellg()};
  file.seekg(0, std::ios::beg);
  file_size -= file.tellg();

  buffer.resize(file_size);
  file.read(&buffer[0], file_size);
  file.close();

  return std::make_tuple(std::move(buffer), true);
}


} // namespace file_util

} // namespace knight
