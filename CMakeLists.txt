cmake_minimum_required(VERSION 3.15)
project(TDR2tree VERSION 1.5.0 LANGUAGES CXX)

option(ENABLE_DOC "Generates the documentation target" OFF)
option(ENABLE_COVERAGE "Generates the coverage build" OFF)
option(ENABLE_TESTING "Turns on testing" OFF)
option(ENABLE_LOGGING "Turn on additional logging" OFF)
option(ENABLE_MT_FILL "Enable experimental multi-threading fill to root file" OFF)
option(ENABLE_POSTGRESQL, "Enable experimental support for filling PostgreSQL" OFF)

#Make sure that custom modules are found
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

##############################################
# Declare dependencies

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})

find_package(ROOT 6.16 CONFIG REQUIRED COMPONENTS RIO Net Hist)

find_package(ZLIB)
if ( ZLIB_FOUND )
    set(ZLIB_flag 1)
else()
    set(ZLIB_flag 0)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

##############################################
# Download and unpack the project

if( ENABLE_LOGGING )
    configure_file(${CMAKE_SOURCE_DIR}/cmake/spdlog-CMakeLists.txt.in ${CMAKE_BINARY_DIR}/spdlog-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/spdlog-download)

    if (result)
        message(FATAL_ERROR "CMake step for spdlog failed: ${result}")
    endif ()

    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/spdlog-download)
    if (result)
        message(FATAL_ERROR "Build step for spdlog failed: ${result}")
    endif ()

    add_subdirectory(${CMAKE_BINARY_DIR}/spdlog-src ${CMAKE_BINARY_DIR}/spdlog-build EXCLUDE_FROM_ALL)
    find_package(spdlog REQUIRED)
endif()

if( ENABLE_POSTGRESQL )
    configure_file(${CMAKE_SOURCE_DIR}/cmake/taopq-CMakeLists.txt.in ${CMAKE_BINARY_DIR}/taopq-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/taopq-download)

    if ( result )
        message(FATAL_ERROR "CMake step for taopq failed: ${result}")
    endif()

    execute_process(COMMAND ${CMAKE_COMMAND} --build -j8 .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/taopq-download)
    if ( result )
        message(FATAL_ERROR "Build step for taopq failed: ${result}")
    endif ()

    add_subdirectory(${CMAKE_BINARY_DIR}/taopq-src ${CMAKE_BINARY_DIR}/taopq-build EXCLUDE_FROM_ALL)
    find_package(taopq REQUIRED)
endif()

##############################################
# Create targets and set properties

add_library(Utilities STATIC
        src/Utilities/ProgressUI.cpp src/Utilities/CLI_interface.cpp)

add_library(Sort::Utilities ALIAS Utilities)

target_include_directories(Utilities
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Utilities
        ${CMAKE_SOURCE_DIR}/external
)

target_compile_features(Utilities PRIVATE cxx_std_11)

add_library(Buffer STATIC
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
        PUBLIC
            Sort::Utilities
)

add_library(Parser STATIC
        src/Parser/TDRparser.cpp
        src/Parser/Parser.cpp src/Parser/TDRtypes.cpp)

add_library(Sort::Parser ALIAS Parser)

target_include_directories(Parser
        PUBLIC
            $<INSTALL_INTERFACE:include>
            $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        PRIVATE
            ${CMAKE_SOURCE_DIR}/src/Parser
            ${CMAKE_SOURCE_DIR}/external
            spdlog::spdlog
)

target_compile_features(Parser PRIVATE cxx_std_11)
target_compile_options(Parser PRIVATE -Wall)

if( ENABLE_LOGGING )
    target_link_libraries(Parser spdlog::spdlog)
    target_compile_definitions(Parser PRIVATE LOG_ENABLED=1)
endif()

add_library(Parameter STATIC
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

target_link_libraries(Parser Sort::Parameter)

add_library(Event STATIC
    src/Event/iThembaEvent.cpp src/Event/iThembaEventBuilder.cpp src/Event/iTLEvent.cpp)

add_library(Sort::Event ALIAS Event)

target_include_directories(Event
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/Event
        ${CMAKE_SOURCE_DIR}/external
)

target_compile_features(Event PRIVATE cxx_std_11)

target_link_libraries(Event PRIVATE Sort::Parameter Sort::Parser ROOT::Tree ROOT::Hist)

add_library(RootInterface STATIC
    src/RootInterface/HistManager.cpp
    src/RootInterface/RootFileManager.cpp
    src/RootInterface/RootInterface.cpp
    src/RootInterface/TreeManager.cpp
)

add_library(Sort::RootInterface ALIAS RootInterface)

target_include_directories(RootInterface
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/RootInterface
)

target_compile_features(RootInterface PRIVATE cxx_std_11)
target_link_libraries(RootInterface ROOT::RIO ROOT::Hist ROOT::Tree Sort::Parameter)

add_executable(TDR2tree ${CMAKE_SOURCE_DIR}/app/TDR2tree.cpp app/SortUtillities.cpp app/SortUtillities.h)

target_include_directories(TDR2tree
    PRIVATE
        ${CMAKE_SOURCE_DIR}/app
        ${ROOT_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/external)

target_compile_features(TDR2tree PRIVATE cxx_std_11)

if ( ENABLE_MT_FILL )
    target_compile_definitions(RootInterface PRIVATE ROOT_MT_FLAG=1)
    target_compile_definitions(TDR2tree PRIVATE ROOT_MT_FLAG=1)
endif()

if ( ENABLE_POSTGRESQL )
    #target_compile_definitions(TDR2tree PRIVATE POSTGRESQL_ENABLED=1)
endif()

if ( ENABLE_LOGGING )
    target_compile_definitions(TDR2tree PRIVATE LOG_ENABLED=1)
endif()

target_link_libraries(TDR2tree
        Sort::Buffer
        Sort::Parser
        Sort::Parameter
        Sort::Utilities
        Sort::Event
        Sort::RootInterface
        ROOT::RIO
        ROOT::Hist
        ROOT::Tree)


##############################################
# Test instructions

if( ENABLE_TESTING )
    enable_testing()
    add_subdirectory(test)
endif ()