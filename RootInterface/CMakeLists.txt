#cmake_minimum_required(VERSION 3.14)
#project(TDR2tree)
#
#
## You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
##   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
##   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
#list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
#
#
#find_package(ROOT REQUIRED COMPONENTS RIO Net)
#include(${ROOT_USE_FILE})
#
#INCLUDE_DIRECTORIES(
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
#)
#
#add_library(RootInterface
#        HistManager.cpp
#        RootFileManager.cpp
#        RootInterface.cpp
#        TreeManager.cpp
#        HistManager.h
#        RootFileManager.h
#        RootInterface.h
#        TreeManager.h)
#
#target_link_libraries(RootInterface ${ROOT_LIBRARIES})

target_sources(TDR2tree PUBLIC
         HistManager.cpp
         RootFileManager.cpp
         RootInterface.cpp
         TreeManager.cpp
         HistManager.h
         RootFileManager.h
         RootInterface.h
         TreeManager.h)