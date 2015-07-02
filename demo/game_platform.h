#pragma once

#include "shader_types.h"
#include "game_memory_types.h"

#include <windows.h>

class GLFWwindow;

#define GAME_INIT(name) void name(knight::GameMemory *game_memory, GLFWwindow &window)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name(knight::GameMemory *game_memory)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_SHUTDOWN(name) void name()
typedef GAME_SHUTDOWN(game_shutdown);

