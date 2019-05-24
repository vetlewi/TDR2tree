cmake_minimum_required(VERSION 3.14)
project(TDR2tree)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native")
set(ROOT_stuff "-L/Users/vetlewi/prog/lib -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -lpthread -Wl,-rpath,/Users/vetlewi/prog/lib -stdlib=libc++ -lm -ldl")

include_directories(./ RootInterface /Users/vetlewi/prog/include)

add_executable(TDR2tree
        BasicStruct.cpp
        Calibration.cpp
        CommandLineInterface.cpp
        experimentsetup.c
        FileReader.cpp
        main.cpp
        Parameters.cpp
        tools.cpp
        XIA_CFD.c
        BasicStruct.h
        Calibration.h
        CommandLineInterface.h
        experimentsetup.h
        FileReader.h
        Parameters.h
        tools.h
        XIA_CFD.h Event.cpp Event.h)

add_subdirectory(RootInterface)
target_link_libraries(TDR2tree ${ROOT_stuff})