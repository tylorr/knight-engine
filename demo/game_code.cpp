#include "game_code.h"
#include "game_platform.h"
#include "file_util.h"
#include "string_util.h"

#include <windows.h>

using namespace knight;
using namespace string_util;

namespace game_code_internal {
  FILETIME WindowsFileTime(uint64_t file_time) {
    ULARGE_INTEGER large_file_time;
    large_file_time.QuadPart = file_time;

    FILETIME windows_file_time;
    windows_file_time.dwLowDateTime = large_file_time.LowPart;
    windows_file_time.dwHighDateTime = large_file_time.HighPart;

    return windows_file_time;
  }
}

namespace game_code {

  void Load(GameCode &game_code, const char *source_dll_name, const char *temp_dll_name) {
    game_code.source_dll_name_ = source_dll_name;
    game_code.temp_dll_name_ = temp_dll_name;

    auto wide_src_buffer = Widen(game_code.allocator_, source_dll_name);
    auto wide_temp_buffer = Widen(game_code.allocator_, temp_dll_name);
    auto wide_src_dll_name = c_str(wide_src_buffer);
    auto wide_temp_dll_name = c_str(wide_temp_buffer);

    CopyFile(wide_src_dll_name, wide_temp_dll_name, false);

    game_code.module_ = LoadLibrary(wide_temp_dll_name);
    game_code.last_write_time_ = file_util::GetLastWriteTime(source_dll_name);

    bool is_valid = false;
    if (game_code.module_) {
      game_code.Init = (game_init *)GetProcAddress(game_code.module_, "Init");
      game_code.UpdateAndRender = (game_update_and_render *)GetProcAddress(game_code.module_, "UpdateAndRender");
      game_code.Shutdown = (game_shutdown *)GetProcAddress(game_code.module_, "Shutdown");
      is_valid = game_code.Init && game_code.UpdateAndRender && game_code.Shutdown;
    }

    if (!is_valid) {
      game_code.Init = nullptr;
      game_code.UpdateAndRender = nullptr;
      game_code.Shutdown = nullptr;
    }
  }

  void Unload(GameCode &game_code) {
    if (game_code.module_) {
      FreeLibrary(game_code.module_);
      game_code.module_ = nullptr;
    }

    game_code.Init = nullptr;
    game_code.UpdateAndRender = nullptr;
    game_code.Shutdown = nullptr;
  }

  bool IsDirty(const GameCode &game_code) {
    auto new_dll_write_time = game_code_internal::WindowsFileTime(
        file_util::GetLastWriteTime(game_code.source_dll_name_));

    auto last_windows_write_time = game_code_internal::WindowsFileTime(game_code.last_write_time_);
    return CompareFileTime(&new_dll_write_time, &last_windows_write_time) != 0;
  }

  void Reload(GameCode &game_code) {
    Unload(game_code);
    Load(game_code, game_code.source_dll_name_, game_code.temp_dll_name_);
  }

} // namespace game_code
