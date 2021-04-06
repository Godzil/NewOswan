set(SOURCES nec.c)
set(HEADERS nec.h necea.h necinstr.h necintrf.h necmodrm.h)

add_library(nec_v30 ${SOURCES} ${HEADERS})

target_include_directories(nec_v30 PUBLIC . ..)