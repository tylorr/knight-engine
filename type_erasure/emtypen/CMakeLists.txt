project(emtypen)

find_package(LibClang REQUIRED)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${LIBCLANG_INCLUDE_DIRS})

add_definitions(-DRELATIVE_DATA_DIR=\"\")

add_executable(emtypen emtypen.cpp)
target_link_libraries(emtypen ${LIBCLANG_LIBRARIES})

if (CMAKE_RUNTIME_OUTPUT_DIRECTORY)
  set(output_path "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
endif()

configure_file(headers.hpp "${output_path}headers.hpp" COPYONLY)
configure_file(form.hpp "${output_path}form.hpp" COPYONLY)
