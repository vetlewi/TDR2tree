

#target_sources(TDRtools
target_sources(TDR2tree
        PUBLIC
            RootFileManager.cpp
            TreeManager.cpp
            HistManager.cpp
        PRIVATE
            RootFileManager.h
            TreeManager.h
            HistManager.h
        )