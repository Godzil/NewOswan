set(SOURCES nec.c nec_debugger.c)
set(HEADERS ../includes/nec.h necea.h necinstr.h ../includes/necintrf.h necmodrm.h ../includes/nec_debugger.h)

add_library(nec_v30 ${SOURCES} ${HEADERS})

target_include_directories(nec_v30 PUBLIC ../includes/)