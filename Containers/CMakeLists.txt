#cmake_minimum_required(VERSION 3.14)
#project(TDR2tree)

#INCLUDE_DIRECTORIES(
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
#)


#add_library(Containers BasicStruct.cpp TDREvent.cpp BasicStruct.h Event.h TDREvent.h)

target_sources(TDR2tree PRIVATE BasicStruct.cpp TDREvent.cpp BasicStruct.h Event.h TDREvent.h)