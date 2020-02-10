set(SOURCES audio.cpp emulate.cpp gpu.cpp io.cpp log.cpp memory.cpp rom.cpp ws.cpp)
set(HEADERS audio.h emulate.h gpu.h ieeprom.h initialIo.h io.h log.h memory.h rom.h SDLptc.h ws.h)

add_library(wswan ${SOURCES} ${HEADERS})

target_link_libraries(wswan nec_v30)

target_include_directories(wswan PUBLIC .)
target_include_directories(wswan PUBLIC ${SDL_INCLUDE_DIR})

add_subdirectory(nec)