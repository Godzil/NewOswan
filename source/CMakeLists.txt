set(SOURCES audio.c emulate.c gpu.c io.c log.c memory.c rom.c ws.c)
set(HEADERS includes/audio.h includes/emulate.h includes/gpu.h includes/io.h includes/log.h includes/memory.h includes/rom.h includes/ws.h)

option(FAKE_DISPLAY "Disable OpenGL and fake displaying" OFF)

add_library(wswan ${SOURCES} ${HEADERS})

if (FAKE_DISPLAY)
    target_compile_options(wswan PRIVATE -DPRETENT_DISPLAY)
endif()

target_link_libraries(wswan nec_v30 glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})

target_include_directories(wswan PUBLIC includes/)

add_subdirectory(nec)