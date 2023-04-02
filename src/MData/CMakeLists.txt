

set(MData_HEADERS
    Allocator.hpp
    AllocatorConcepts.hpp
    BitArray.hpp
    BucketArray.hpp
    Pointers.hpp
    Vector.hpp
)

set(MData_SOURCES
)


add_library(MData INTERFACE ${MData_SOURCES} ${MData_HEADERS})
# target_include_directories(MData PRIVATE "${PROJECT_SOURCE_DIR}/src")

target_link_libraries(MData INTERFACE MUtils)

add_executable(MData_Test main.cpp)
target_link_libraries(MData_Test MData)