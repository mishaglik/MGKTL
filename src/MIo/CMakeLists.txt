

set(MIO_HEADERS
    print.hpp
    stream.hpp
)

set(MIO_SOURCES
    stream.cpp
)


add_library(MIo)
target_sources(MIo PRIVATE ${MIO_SOURCES} ${MIO_HEADERS})
target_include_directories(MIo PUBLIC "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(MIo PRIVATE MUtils)

add_executable(MIo_Test test.cpp)
target_link_libraries(MIo_Test MIo)