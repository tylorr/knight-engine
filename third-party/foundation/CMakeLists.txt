project(foundation)

include(cotire)

set(SOURCES
    memory.cpp
    murmur_hash.cpp
    string_stream.cpp
    dlmalloc.cpp
)

add_library(foundation ${SOURCES})
cotire(foundation)

enable_testing()
add_executable(foundation_test unit_test.cpp ${SOURCES})
add_test(NAME foundation_test_all COMMAND foundation_test)
