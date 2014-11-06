project(emtypen)

find_package(LibClang REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${LIBCLANG_INCLUDE_DIRS})

add_definitions(-DRELATIVE_DATA_DIR=\"\")

add_executable(emtypen emtypen.cpp)
target_link_libraries(emtypen ${LIBCLANG_LIBRARIES})

cotire(emtypen)

configure_file(headers.hpp headers.hpp COPYONLY)
configure_file(form.hpp form.hpp COPYONLY)
