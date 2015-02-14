#include "shader_types.h"

class GLFWwindow;

namespace knight {

#define GAME_INIT(name) void name(GLFWwindow &window)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name()
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_SHUTDOWN(name) void name()
typedef GAME_SHUTDOWN(game_shutdown);

} // namespace knight
