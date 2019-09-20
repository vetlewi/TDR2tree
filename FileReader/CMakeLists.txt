#cmake_minimum_required(VERSION 3.14)
#project(TDR2tree)
#
#
#INCLUDE_DIRECTORIES(
#        ${CMAKE_CURRENT_SOURCE_DIR}
#        ${CMAKE_CURRENT_SOURCE_DIR}/..
#)
#
#add_library(BufferFetcher
#        FileReader.cpp
#        MTFileBufferFetcher.cpp
#        STFileBufferFetcher.cpp
#        TDRFileReader.cpp
#        BufferFetcher.h
#        BufferType.h
#        FileBufferFetcher.h
#        FileReader.h
#        MTFileBufferFetcher.h
#        STFileBufferFetcher.h
#        TDRFileReader.h)

target_sources(TDR2tree PRIVATE
        FileReader.cpp
         MTFileBufferFetcher.cpp
         STFileBufferFetcher.cpp
         TDRFileReader.cpp
         BufferFetcher.h
         BufferType.h
         FileBufferFetcher.h
         FileReader.h
         MTFileBufferFetcher.h
         STFileBufferFetcher.h
         TDRFileReader.h)