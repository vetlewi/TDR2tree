project(external C CXX)

set(BUILD_TESTING_BCKP ${BUILD_TESTING})
set(BUILD_TESTING OFF CACHE BOOL "Force disable of tests for external dependencies" FORCE)

set(CUSTOM_COVERAGE_EXCLUDE ${CUSTOM_COVERAGE_EXCLUDE} "external" PARENT_SCOPE) # Replaced in CTestCustom.cmake.in

add_subdirectory(CLI11 EXCLUDE_FROM_ALL)
add_subdirectory(readerwriterqueue EXCLUDE_FROM_ALL)
add_subdirectory(concurrentqueue EXCLUDE_FROM_ALL)

#Use spdlog for logging, and tell it to use our version of fmtlib
add_subdirectory(spdlog EXCLUDE_FROM_ALL)

#Doctest for unit tests
add_library(doctest INTERFACE)
target_include_directories(
        doctest
        INTERFACE
        doctest/doctest # note : will expose the parts/ folder...
)
add_library(doctest::doctest ALIAS doctest)
target_compile_features(doctest INTERFACE cxx_std_11)

set(BUILD_TESTING ${BUILD_TESTING_BCKP} CACHE BOOL "Build tests (default variable for CTest)" FORCE) #Set it back to its past value