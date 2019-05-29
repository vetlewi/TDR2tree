

#target_sources(TDRtools
target_sources(TDR2tree
        PRIVATE
            Calibration.cpp
            experimentsetup.c
            Parameters.cpp
        PUBLIC
            Calibration.h
            experimentsetup.h
            Parameters.h
        )