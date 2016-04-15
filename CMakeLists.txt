cmake_minimum_required(VERSION 2.8)
project(SoundClass)

SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})

find_package(qibuild)
find_path(LIB_PATH libxtract.a)

option(PC_SIDE
  "ON - OFF"
  ON)

set(CMAKE_CXX_FLAGS "-std=c++0x")

if(PC_SIDE)
   add_definitions( " -DPC_SIDE ")
   qi_create_bin(SCModule "SCmain.cpp" "features.cpp" "features.h" "SCModule.cpp" "SCModule.h" "config.h" "noiseFilter.h" "noiseFilter.cpp" "fft_ifft.h")
   qi_create_bin(Listener "listener.cpp" "config.h" "noiseFilter.h" "noiseFilter.cpp" "fft_ifft.h")
   qi_create_bin(Learner "learner.cpp" "features.cpp" "features.h" "config.h" "noiseFilter.h" "noiseFilter.cpp" "fft_ifft.h")
   qi_create_bin(fileClassify "fileClassify.cpp" "config.h" "features.cpp" "features.h")
   qi_create_bin(naoClassify "naoClassify.cpp" "config.h")
   qi_create_bin(Configurator "configurator.cpp")

   qi_use_lib(Learner XTRACT SNDFILE OPENCV2_CORE OPENCV2_ML)
   qi_use_lib(Listener SNDFILE ALCOMMON ALPROXIES)
   qi_use_lib(SCModule XTRACT ALCOMMON ALAUDIO ALPROXIES BOOST OPENCV2_CORE OPENCV2_ML)
   qi_use_lib(fileClassify XTRACT SNDFILE OPENCV2_CORE OPENCV2_ML)
   qi_use_lib(naoClassify ALCOMMON ALPROXIES)

else()
   qi_create_lib(SCModule "SCmain.cpp" "features.cpp" "features.h" "SCModule.cpp" "SCModule.h" "config.h" "noiseFilter.h" "noiseFilter.cpp" "fft_ifft.h" SUBFOLDER naoqi)
   qi_create_bin(naoClass "naoClassify.cpp" "config.h" SUBFOLDER naoqi)
   qi_create_bin(configurator "configurator.cpp" SUBFOLDER naoqi)

   qi_use_lib(SCModule XTRACT ALCOMMON ALAUDIO ALPROXIES BOOST OPENCV2_CORE OPENCV2_ML ALTHREAD)
   qi_use_lib(naoClass ALCOMMON ALPROXIES)
endif()



