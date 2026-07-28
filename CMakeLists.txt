cmake_minimum_required (VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" ".lib" ".so" ".dll")

project ("stack-challenge21")




add_executable (main "src/main.cpp")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(main PRIVATE -static)
endif()


