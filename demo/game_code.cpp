#include "game_code.h"
#include "game_platform.h"

#include <logog.hpp>
#include <boost/filesystem/operations.hpp>

namespace fs = boost::filesystem;

namespace game_code {

  void load(GameCode &game_code, fs::path source_dll_path, fs::path temp_dll_path) {
    game_code.source_dll_path_ = source_dll_path;
    game_code.temp_dll_path_ = temp_dll_path;

    fs::copy_file(source_dll_path, temp_dll_path, fs::copy_option::overwrite_if_exists);

    game_code.module_ = LoadLibrary(temp_dll_path.c_str());
    game_code.last_write_time_ = fs::last_write_time(source_dll_path);

    bool is_valid = false;
    if (game_code.module_) {
      game_code.Init = reinterpret_cast<game_init *>(GetProcAddress(game_code.module_, "Init"));
      game_code.UpdateAndRender = reinterpret_cast<game_update_and_render *>(GetProcAddress(game_code.module_, "UpdateAndRender"));
      game_code.Shutdown = reinterpret_cast<game_shutdown *>(GetProcAddress(game_code.module_, "Shutdown"));
      is_valid = game_code.Init && game_code.UpdateAndRender && game_code.Shutdown;
    } else {
      WARN("Could not load module");
    }

    if (!is_valid) {
      WARN("Could not load game code");
      game_code.Init = nullptr;
      game_code.UpdateAndRender = nullptr;
      game_code.Shutdown = nullptr;
    }
  }

  void unload(GameCode &game_code) {
    if (game_code.module_) {
      FreeLibrary(game_code.module_);
      game_code.module_ = nullptr;
    }

    game_code.Init = nullptr;
    game_code.UpdateAndRender = nullptr;
    game_code.Shutdown = nullptr;
  }

  bool is_dirty(const GameCode &game_code) {
    auto current_write_time = fs::last_write_time(game_code.source_dll_path_);
    return std::difftime(current_write_time, game_code.last_write_time_) > 0;
  }

  void reload(GameCode &game_code) {
    unload(game_code);
    load(game_code, game_code.source_dll_path_, game_code.temp_dll_path_);
  }

} // namespace game_code
