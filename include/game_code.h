#pragma once

#include "platform_types.h"

namespace knight {

namespace game_code {

  GameCode Load(const char *source_dll_name, const char *temp_dll_name);
  void Unload(GameCode &game_code);

  bool IsDirty(const GameCode &game_code);
  GameCode Reload(GameCode &game_code);

} // namespace game_code

} // namespace knight
