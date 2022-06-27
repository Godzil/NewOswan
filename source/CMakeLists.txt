set(SOURCES emulate.c gpu.c io.c log.c memory.c rom.c ws.c file_access.c)
set(PERIPHERAL_SOURCES peripherals/audio.c peripherals/buttons.c peripherals/color_gpu.c peripherals/color_system.c
        peripherals/debug.c peripherals/dma.c peripherals/eeprom.c peripherals/mono_gpu.c peripherals/mono_system.c
        peripherals/rtc.c peripherals/universal_luxor.c)
set(HEADERS includes/audio.h includes/device.h includes/emulate.h includes/gpu.h includes/io.h includes/log.h
        includes/memory.h includes/nec.h includes/nec_debugger.h includes/necintrf.h includes/rom.h includes/ws.h
        includes/file_access.h)

option(FAKE_DISPLAY "Disable OpenGL and fake displaying" OFF)

add_library(wswan ${SOURCES} ${PERIPHERAL_SOURCES} ${HEADERS})

if (FAKE_DISPLAY)
    target_compile_options(wswan PRIVATE -DPRETENT_DISPLAY)
endif()

target_link_libraries(wswan nec_v30 glfw ${OPENGL_glu_LIBRARY} ${OPENGL_gl_LIBRARY})

target_include_directories(wswan PUBLIC includes/)

add_subdirectory(nec)