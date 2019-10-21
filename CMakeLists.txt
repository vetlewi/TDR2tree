cmake_minimum_required(VERSION 3.14)
project(TDR2tree VERSION 1.5.0 LANGUAGES CXX)

option(ENABLE_DOC "Generates the documentation target" OFF)
option(ENABLE_COVERAGE "Generates the coverage build" OFF)
option(ENABLE_TESTING "Turns on testing" OFF)
set(ENABLE_TESTING 1)

#Make sure that custom modules are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

##############################################
# Declare dependencies

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT REQUIRED COMPONENTS RIO Net)

find_package(ZLIB)
if ( ZLIB_FOUND )
    set(ZLIB_flag 1)
else()
    set(ZLIB_flag 0)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

##############################################
# Create targets and set properties

add_library(Buffer SHARED
        src/Buffer/aptr.cpp
        src/Buffer/FileReader.cpp
        src/Buffer/STFileBufferFetcher.cpp
        src/Buffer/MTFileBufferFetcher.cpp
        src/Buffer/PrefetchThread.cpp)

add_library(Sort::Buffer ALIAS Buffer)

target_include_directories(Buffer
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/src/Buffer
            ${ZLIB_INCLUDE_DIRS}
)

target_compile_features(Buffer PRIVATE cxx_std_11)
target_compile_definitions(Buffer PRIVATE HAVE_ZLIB=${ZLIB_flag})

target_link_libraries(Buffer
        PRIVATE
            ZLIB::ZLIB
            Threads::Threads
)

add_library(Parser SHARED
        src/Parser/TDRparser.cpp
)

add_library(Sort::Parser ALIAS Parser)

target_include_directories(Parser
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/Parser
            ${CMAKE_SOURCE_DIR}/external
)

target_compile_features(Parser PRIVATE cxx_std_11)

add_library(Parameter SHARED
        src/Parameters/Calibration.cpp
        src/Parameters/experimentsetup.cpp
        src/Parameters/Parameters.cpp
        src/Parameters/XIA_CFD.cpp
)

add_library(Sort::Parameter ALIAS Parameter)

target_include_directories(Parameter
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Parser
        ${CMAKE_SOURCE_DIR}/external
)

target_compile_features(Parameter PRIVATE cxx_std_11)


add_executable(TDR2tree main.cpp)
target_link_libraries(TDR2tree Sort::Buffer Sort::Parser Sort::Parameter)


##############################################
# Test instructions

if( ENABLE_TESTING )
    enable_testing()
    add_subdirectory(test)
endif ()