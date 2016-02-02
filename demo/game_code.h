#pragma once

#include "game_platform.h"

#include <memory_types.h>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <ctime>
#include <windows.h>

struct GameCode {
  GameCode(foundation::Allocator &allocator)
    : allocator_{allocator} { }

  game_init *Init;
  game_update_and_render *UpdateAndRender;
  game_shutdown *Shutdown;

  boost::filesystem::path source_dll_path_;
  boost::filesystem::path temp_dll_path_;

  HMODULE module_;
  std::time_t last_write_time_;

  foundation::Allocator &allocator_;
};

namespace game_code {

  void load(GameCode &game_code, boost::filesystem::path source_dll_path, boost::filesystem::path temp_dll_path);
  void unload(GameCode &game_code);

  bool is_dirty(const GameCode &game_code);
  void reload(GameCode &game_code);

} // namespace game_code
