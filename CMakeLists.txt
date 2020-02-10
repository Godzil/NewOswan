cmake_minimum_required(VERSION 3.1)

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})

project("NewOswan")

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

find_package(SDL 1.2 REQUIRED)

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Werror")

message("-- Building version ${VERSION}")

add_executable(wonderswan main.cpp)

target_compile_definitions(wonderswan PUBLIC VERSION="${VERSION}")
target_include_directories(wonderswan PUBLIC source)
target_include_directories(wonderswan PUBLIC ${SDL_INCLUDE_DIR})

add_subdirectory(source)

target_link_libraries(wonderswan wswan ${SDL_LIBRARY})