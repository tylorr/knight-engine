namespace knight {

class UniformFactory;

#define GAME_INIT(name) void name(UniformFactory &uniform_factory)
typedef GAME_INIT(game_init);

#define GAME_UPDATE_AND_RENDER(name) void name()
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_SHUTDOWN(name) void name()
typedef GAME_SHUTDOWN(game_shutdown);

} // namespace knight
