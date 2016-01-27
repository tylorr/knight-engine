#pragma once

#include "game_platform.h"

#include <memory_types.h>

#include <cstdint>
#include <windows.h>

struct GameCode {
  GameCode(foundation::Allocator &allocator)
    : allocator_{allocator} { }

  game_init *Init;
  game_update_and_render *UpdateAndRender;
  game_shutdown *Shutdown;

  const char *source_dll_name_;
  const char *temp_dll_name_;

  HMODULE module_;
  uint64_t last_write_time_;

  foundation::Allocator &allocator_;
};

namespace game_code {

  void Load(GameCode &game_code, const char *source_dll_name, const char *temp_dll_name);
  void Unload(GameCode &game_code);

  bool IsDirty(const GameCode &game_code);
  void Reload(GameCode &game_code);

} // namespace game_code
