#pragma once

#include "common.h"

#include <string_stream.h>

#include <collection_types.h>
#include <windows.h>

namespace knight {

struct FileRead {
  FileRead(const char *path);
  FileRead(FileRead &&other);
  ~FileRead();

  void Read(foundation::Array<char, 4> &content) const;

  FileRead &operator=(FileRead &&other);

  const char *path_;
  HANDLE file_handle_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(FileRead);
};

struct FileWrite {
  FileWrite(const char *path);
  FileWrite(FileWrite &&other);
  ~FileWrite();

  void Write(const foundation::Array<char, 4> &content) const;
  void Write(const void *data, uint32_t size) const;

  FileWrite &operator=(FileWrite &&other);

  const char *path_;
  HANDLE file_handle_;

 private:
  KNIGHT_DISALLOW_COPY_AND_ASSIGN(FileWrite);
};

struct GameCode {
  game_init *Init;
  game_update_and_render *UpdateAndRender;
  game_shutdown *Shutdown;

  const char *source_dll_name_;
  const char *temp_dll_name_;

  HMODULE module_;
  uint64_t last_write_time_;
};

} // namespace knight
