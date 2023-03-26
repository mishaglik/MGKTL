message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER}")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
elseif("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GCC")
else()
message(FATAL_ERROR "Unsupported compiler")
endif()