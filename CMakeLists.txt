cmake_minimum_required(VERSION 3.14)
project(TDR2tree)

set(CMAKE_CXX_STANDARD 11)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})


find_package(ROOT REQUIRED COMPONENTS RIO Net)
include(${ROOT_USE_FILE})

#find_package(CGAL)
#set(CGAL_DO_NOT_WARN_ABOUT_CMAKE_BUILD_TYPE TRUE)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")

include_directories(. Containers FileReader PhysicalParam RootInterface Tools)

#ROOT_GENERATE_DICTIONARY(TDRtools_dic Containers/Event.h LINKDEF LinkDef.h)

#add_library(TDRtools SHARED main.cpp TDRtools_dic.cxx)
add_executable(TDR2tree main.cpp)

add_subdirectory(Containers)
add_subdirectory(FileReader)
add_subdirectory(PhysicalParam)
add_subdirectory(RootInterface)
add_subdirectory(Tools)

#target_link_libraries(TDRtools ${ROOT_LIBRARIES})
target_link_libraries(TDR2tree ${ROOT_LIBRARIES})
#target_link_libraries(TDR2tree TDRtools)


install(TARGETS TDR2tree DESTINATION /Users/vetlewi/prog/bin)