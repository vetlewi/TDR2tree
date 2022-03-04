cmake_minimum_required(VERSION 3.14)
project(TDR2tree VERSION 0.9.0 LANGUAGES C CXX)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
include(cmake/CPM.cmake)

find_package(Threads REQUIRED)
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

CPMAddPackage(
        NAME indicators
        GITHUB_REPOSITORY p-ranav/indicators
        VERSION 2.2
        OPTIONS "INDICATORS_BUILD_TESTS OFF" "INDICATORS_SAMPLES OFF" "INDICATORS_DEMO OFF"
)

CPMAddPackage(
        NAME fmt
        GITHUB_REPOSITORY fmtlib/fmt
        GIT_TAG 7.1.3
)

CPMAddPackage(
        NAME sanitizers-cmake
        GITHUB_REPOSITORY arsenm/sanitizers-cmake
        GIT_TAG 99e159ec9bc8dd362b08d18436bd40ff0648417b
        DOWNLOAD_ONLY YES
)

if ( sanitizers-cmake_ADDED )
    set(CMAKE_MODULE_PATH "${sanitizers-cmake_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
    find_package(Sanitizers)
endif()



add_library(TDR2treeLib
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Event.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histogram1D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histogram2D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histogram3D.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/Histograms.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/MamaWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/RFTreeEvent.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/RootWriter.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Containers/TreeEvent.cpp

        ${CMAKE_CURRENT_SOURCE_DIR}/source/FileReader/MemoryMap.cpp
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
        ${CMAKE_CURRENT_SOURCE_DIR}/source/Tasks/RootSort.cpp
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

target_link_libraries(TDR2treeLib
    PUBLIC
        Threads::Threads
        ROOT::RIO
        ROOT::Hist
        ROOT::Tree
        readerwriterqueue
        concurrentqueue
        structopt::structopt
        indicators::indicators
        fmt::fmt)

target_compile_features(TDR2treeLib PUBLIC cxx_std_17)

add_executable(TDR2tree app/main.cpp)
add_executable(TDR2treeMT app/mt_main.cpp)
add_executable(TDR2treeST app/st_main.cpp)

target_link_libraries(TDR2tree TDR2treeLib)
target_link_libraries(TDR2treeMT TDR2treeLib readerwriterqueue concurrentqueue)
target_link_libraries(TDR2treeST TDR2treeLib)
