# XTRACT
unset(XTRACT_FOUND)

find_path(XTRACT_INCLUDE_DIR xtract/libxtract.h
  HINTS
  /include
  /local/include)

find_library(XTRACT_LIBRARY NAMES libxtract.a
  HINTS
  /lib
  /local/lib)



set(XTRACT_LIBS ${XTRACT_LIBRARY})
if(XTRACT_LIBS AND XTRACT_INCLUDE_DIR)
    set(XTRACT_FOUND 1)
endif()
