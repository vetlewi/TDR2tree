cmake_minimum_required(VERSION 3.14)
project(TDR2tree VERSION 0.9.0 LANGUAGES C CXX)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
include(cmake/CPM.cmake)

find_package(ROOT REQUIRED COMPONENTS RIO Tree Hist)

CPMAddPackage(
        NAME structopt
        GITHUB_REPOSITORY p-ranav/structopt
        GIT_TAG ffe3473c1cf961ead13eda8e2e4f6e7e79fd9c5c
        OPTIONS "STRUCTOPT_TESTS OFF" "STRUCTOPT_SAMPLES OFF")

CPMAddPackage(
        NAME readerwriterqueue
        GITHUB_REPOSITORY cameron314/readerwriterqueue
        GIT_TAG 09732844aff873e8ec489e9005c775b6b1c58fa2
)

CPMAddPackage(
        NAME concurrentqueue
        GITHUB_REPOSITORY cameron314/concurrentqueue
        VERSION 1.0.3
)

add_library(TDR2treeLib
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histogram1D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histogram2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histograms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/TreeEvent.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/BasicFileReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/FileReader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/MemoryMap.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/MTFileBufferFetcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/STFileBufferFetcher.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/TDRChannel.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/TDREntry.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/TDRParser.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/TDRTypes.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/XIA_CFD.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/PhysicalParam/Calibration.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/PhysicalParam/experimentsetup.c
        ${CMAKE_CURRENT_SOURCE_DIR}/source/PhysicalParam/Parameters.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/RootInterface/HistManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/RootInterface/RootFileManager.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/RootInterface/TreeManager.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Buffer.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Converter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Reader.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Sort.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Splitter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/Trigger.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tools/CommandLineInterface.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tools/ProgressUI.cpp)

target_include_directories(TDR2treeLib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Containers
        ${CMAKE_CURRENT_SOURCE_DIR}/include/FileReader
        ${CMAKE_CURRENT_SOURCE_DIR}/include/PhysicalParam
        ${CMAKE_CURRENT_SOURCE_DIR}/include/RootInterface
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Tasks
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Tools
)

target_link_libraries(TDR2treeLib PUBLIC ROOT::RIO ROOT::Hist ROOT::Tree readerwriterqueue concurrentqueue)
target_compile_features(TDR2treeLib PUBLIC cxx_std_17)

add_executable(TDR2tree app/main.cpp)
add_executable(TDR2treeMT app/mt_main.cpp)
add_executable(TDR2treeST app/st_main.cpp)

target_link_libraries(TDR2tree TDR2treeLib structopt::structopt)
target_link_libraries(TDR2treeMT TDR2treeLib readerwriterqueue concurrentqueue)
target_link_libraries(TDR2treeST TDR2treeLib readerwriterqueue)