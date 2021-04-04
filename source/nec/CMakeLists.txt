set(SOURCES nec.cpp)
set(HEADERS nec.h necea.h necinstr.h necintrf.h necmodrm.h)

add_library(nec_v30 ${SOURCES} ${HEADERS})