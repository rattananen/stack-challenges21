cmake_minimum_required (VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)

project ("stack-challenge25")

function(Add_copy_asset for)
    set(resource_dir "${CMAKE_SOURCE_DIR}/assets/")
    set(resource_dest_dir "${CMAKE_CURRENT_BINARY_DIR}/") 
    add_custom_command(
         TARGET ${for} POST_BUILD
         COMMAND ${CMAKE_COMMAND} -E copy_directory ${resource_dir} ${resource_dest_dir} 
         COMMENT "copy_directory assets folder from ${resource_dir} => ${resource_dest_dir}"
     )
endfunction()

find_package(nlohmann_json CONFIG REQUIRED)

add_executable (main "src/main.cpp")
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_link_options(main PRIVATE -static)
endif()

Add_copy_asset(main)


