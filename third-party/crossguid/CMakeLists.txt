project(crossguid)

add_definitions(-DGUID_WINDOWS)
add_library(crossguid guid.cpp)
