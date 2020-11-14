cmake_minimum_required(VERSION 3.1)

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})

project("NewOswan")

include(GetGitRevisionDescription)
git_describe(VERSION --tags --dirty=-dirty)

# Include GLFW3
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("external/glfw")
find_package(OpenGL REQUIRED)

include_directories(${OPENGL_INCLUDE_DIR})

set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-unused-result -Wno-write-strings -Werror")

message("-- Building version ${VERSION}")

add_executable(wonderswan main.cpp)

set_property(TARGET wonderswan PROPERTY CXX_STANDARD 98)

target_compile_definitions(wonderswan PUBLIC VERSION="${VERSION}")
target_include_directories(wonderswan PUBLIC source)

add_subdirectory(source)

target_link_libraries(wonderswan wswan glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})

add_executable(dumpinfo dumpinfo.c)