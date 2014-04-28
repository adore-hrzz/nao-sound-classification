# Copyright (C) 2010 Aldebaran Robotics

cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)
project(KlasifikacijaGovora)
find_package(qibuild)

option(KLASIFIKACIJAGOVORA_IS_REMOTE
  "module is compiled as a remote module (ON or OFF)"
  OFF)

set(_srcs
    main.cpp
    KlasifikacijaGovora.h
    KlasifikacijaGovora.cpp
)

if(KLASIFIKACIJAGOVORA_IS_REMOTE)
  add_definitions(" -DKLASIFIKACIJAGOVORA_IS_REMOTE ")
  qi_create_bin(KlasifikacijaGovora ${_srcs})
else()
  qi_create_lib(KlasifikacijaGovora SHARED ${_srcs} SUBFOLDER naoqi)
endif()


qi_use_lib(KlasifikacijaGovora ALCOMMON ALAUDIO)
