

add_definitions(-Wall -g -fcolor-diagnostics -std=c++11)

set(SOURCES
    # camera.cpp
    # geometry.cpp
    # gradient_map_material.cpp
    main.cpp
    # material.cpp
    # script.cpp
    # shader_cache.cpp
    # transform.cpp
    # utils.cpp
)

add_executable(game-engine ${SOURCES})

find_package(GLEW REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/third-party/glfw/include)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/third-party/glm)
include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(game-engine glfw ${GLFW_LIBRARIES})
target_link_libraries(game-engine stb_image)
target_link_libraries(game-engine ${GLEW_LIBRARIES})
