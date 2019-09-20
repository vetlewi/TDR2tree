#cmake_minimum_required(VERSION 3.14)
#project(TDR2tree)
#
#INCLUDE_DIRECTORIES(
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
#)
#
#add_library(PhysicalParam
#        Calibration.cpp
#        experimentsetup.cpp
#        Parameters.cpp
#        XIA_CFD.c
#        Calibration.h
#        experimentsetup.h
#        Parameters.h
#        XIA_CFD.h)

target_sources(TDR2tree PUBLIC
         Calibration.cpp
         experimentsetup.cpp
         Parameters.cpp
         XIA_CFD.c
         Calibration.h
         experimentsetup.h
         Parameters.h
         XIA_CFD.h)