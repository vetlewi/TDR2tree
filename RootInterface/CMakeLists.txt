

#target_sources(TDRtools
target_sources(TDR2tree
        PRIVATE
            RootInterface.cpp
            RootFileManager.cpp
            HistManager.cpp
        PUBLIC
            RootFileManager.h
            TreeManager.h
            HistManager.h
        INTERFACE
            RootInterface.h
        )