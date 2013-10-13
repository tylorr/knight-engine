set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fcolor-diagnostics -std=c++11")

set(SOURCES
    main.cpp
)

add_executable(demo ${SOURCES})

find_package(GLEW REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/third-party/glfw/include)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/third-party/glm)
# include_directories(${LUA_INCLUDE_DIR})
# include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/include)

target_link_libraries(demo glfw ${GLFW_LIBRARIES})
target_link_libraries(demo ${GLEW_LIBRARIES})
# target_link_libraries(demo stb_image)
target_link_libraries(demo logog)
target_link_libraries(demo knight-engine)
# target_link_libraries(demo ${LUA_LIBRARIES})
