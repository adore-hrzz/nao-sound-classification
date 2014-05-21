# Copyright (C) 2010 Aldebaran Robotics

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)
project(nao-sound-classification)
find_package(qibuild)

option(LRKLASIFIKACIJAZVUKOVA_IS_REMOTE
  "module is compiled as a remote module (ON or OFF)"
  OFF)

set(_srcs
    main.cpp
    KlasifikacijaGovora.h
    KlasifikacijaGovora.cpp
    kiss_fft130/kiss_fft.h
    kiss_fft130/kiss_fft.c
)

if(LRKLASIFIKACIJAZVUKOVA_IS_REMOTE)
  add_definitions(" -DLRKLASIFIKACIJAZVUKOVA_IS_REMOTE ")
  qi_create_bin(LRKlasifikacijaZvukova ${_srcs})
else()
  qi_create_lib(LRKlasifikacijaZvukova SHARED ${_srcs} SUBFOLDER naoqi)
endif()


qi_use_lib(LRKlasifikacijaZvukova ALCOMMON ALAUDIO)
