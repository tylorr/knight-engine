#pragma once

#include "common.h"

#include <string_stream.h>

#include <collection_types.h>
#include <windows.h>

namespace knight {

struct FileRead {
  FileRead(const char *path);

  FileRead(FileRead &&other);
  FileRead &operator=(FileRead &&other);

  ~FileRead();

  void Read(foundation::Array<char, 4> &content) const;

  const char *path_;
  HANDLE file_handle_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(FileRead);
};

struct FileWrite {
  FileWrite(const char *path);
  
  FileWrite(FileWrite &&other);
  FileWrite &operator=(FileWrite &&other);

  ~FileWrite();

  void Write(const foundation::Array<char, 4> &content) const;
  void Write(const void *data, uint32_t size) const;

  const char *path_;
  HANDLE file_handle_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(FileWrite);
};



} // namespace knight
