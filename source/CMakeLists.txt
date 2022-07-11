cmake_minimum_required(VERSION 3.1)
project(libwswan)

file(GLOB SOURCES *.c)
file(GLOB_RECURSE HEADERS includes/*.h)
file(GLOB_RECURSE PERIPHERAL_SOURCES peripherals/*.c)

file(GLOB_RECURSE NEC_SOURCES nec/*.c nec/*.h)
set(NEC_HEADERS includes/nec.h nec/necea.h nec/necinstr.h includes/necintrf.h nec/necmodrm.h includes/nec_debugger.h)

option(FAKE_DISPLAY "Disable OpenGL and fake displaying" OFF)

add_library(wswan)
target_include_directories(wswan PUBLIC includes/)
target_sources(wswan PRIVATE ${SOURCES} ${PERIPHERAL_SOURCES} ${NEC_SOURCES})
target_sources(wswan PRIVATE ${HEADERS})

if (FAKE_DISPLAY)
    target_compile_options(wswan PRIVATE -DPRETENT_DISPLAY)
endif()

target_link_libraries(wswan glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})
