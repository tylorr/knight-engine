#pragma once

struct GLFWwindow;

using game_init = void(GLFWwindow &);
using game_update_and_render = void();
using game_shutdown = void();

