

target_sources(TDR2tree
        PRIVATE
            aptr.ipp
            FileReader.cpp
            MTFileBufferFetcher.cpp
            STFileBufferFetcher.cpp
            TDRFileReader.cpp
            XIA_CFD.c
        PUBLIC
            BufferType.h
            FileBufferFetcher.h
            FileReader.h
            MTFileBufferFetcher.h
            STFileBufferFetcher.h
            TDRFileReader.h
            XIA_CFD.h
        )