set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

set(SOURCES
    main.cpp
)

add_definitions(-DDEVELOPMENT)

find_package(GLEW REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/glfw/include)
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${PROJECT_SOURCE_DIR}/third-party/glm)
include_directories(${PROJECT_SOURCE_DIR}/third-party/logog/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/stb_image)
include_directories(${PROJECT_SOURCE_DIR}/third-party/imgui)
include_directories(${PROJECT_SOURCE_DIR}/third-party/flatbuffers/include)
include_directories(${PROJECT_SOURCE_DIR}/third-party/foundation)
# include_directories(${LUA_INCLUDE_DIR})

add_executable(demo ${SOURCES})
register_event_fbs(demo schema/monster.fbs schema/event_header.fbs)

target_link_libraries(demo knight-engine)
target_link_libraries(demo imgui)
cotire(demo)
