set(SOURCES audio.c emulate.c gpu.c io.c log.c memory.c rom.c ws.c)
set(HEADERS audio.h emulate.h gpu.h io.h log.h memory.h rom.h ws.h)

add_library(wswan ${SOURCES} ${HEADERS})

target_link_libraries(wswan nec_v30 glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})

target_include_directories(wswan PUBLIC .)

add_subdirectory(nec)