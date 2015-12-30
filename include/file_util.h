#pragma once

#include "gsl.h"
#include "types.h"

#include <collection_types.h>
#include <string_stream.h>

#include <cstdint>

namespace knight {

struct DirectoryContent {
  DirectoryContent(foundation::Allocator &alloc) :
    path{alloc} { }

  foundation::string_stream::Buffer path;

  int depth;
};

namespace file_util {

uint64_t GetLastWriteTime(czstring filename);

bool ListDirectoryContents(
  foundation::Allocator &allocator, czstring directory_path);

} // namespace file_util

} // namespace knight
