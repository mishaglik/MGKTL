

set(MContainers_HEADERS
    Treap.hpp
)

set(MContainers_SOURCES
)


add_library(MContainers INTERFACE ${MContainers_SOURCES} ${MContainers_HEADERS})
target_include_directories(MContainers INTERFACE .)

target_link_libraries(MContainers INTERFACE MUtils)

add_executable(MContainers_Test test.cpp)
target_link_libraries(MContainers_Test MContainers MIo)