
SET(MUTILS_HEADERS
    utils.hpp
    function.hpp
    defines.hpp
)

add_library(MUtils INTERFACE)
project_log("Src dir: ${CMAKE_SOURCE_DIR}/src")
target_include_directories(MUtils INTERFACE "${CMAKE_SOURCE_DIR}/src")