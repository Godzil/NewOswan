set(SOURCES nec.c nec_debugger.c)
set(HEADERS nec.h necea.h necinstr.h necintrf.h necmodrm.h nec_debugger.h)

add_library(nec_v30 ${SOURCES} ${HEADERS})

target_include_directories(nec_v30 PUBLIC . ..)