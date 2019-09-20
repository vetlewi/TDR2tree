#cmake_minimum_required(VERSION 3.14)
#project(TDR2tree)
#
#INCLUDE_DIRECTORIES(
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
#)
#
#add_library(Tools CommandLineInterface.cpp ProgressUI.cpp CommandLineInterface.h ProgressUI.h)

target_sources(TDR2tree PUBLIC
        aptr.ipp
        CommandLineInterface.cpp
        ProgressUI.cpp
        CommandLineInterface.h
        ProgressUI.h
        aptr.h)