#pragma once

#include "common.h"

#include <collection_types.h>
#include <windows.h>

namespace knight {

struct File {
  File(const char *path);
  File(File &&other);
  ~File();

  void Read(foundation::Array<char, 4> &content) const;

  File &operator=(File &&other);

  const char *path_;
  HANDLE file_handle_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(File);
};

} // namespace knight
