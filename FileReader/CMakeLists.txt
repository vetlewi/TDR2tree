

target_sources(TDR2tree
        PRIVATE
            aptr.ipp
            FileReader.cpp
            MTFileBufferFetcher.cpp
            STFileBufferFetcher.cpp
            XIA_CFD.cpp
            TDRChannel.cpp
            TDRParser.cpp
            TDRTypes.cpp
            TDREntry.cpp
            MemoryMap.cpp
        PUBLIC
            BufferType.h
            TDRChannel.h
            TDRParser.h
            TDRTypes.h
            TDREntry.h
            FileBufferFetcher.h
            FileReader.h
            MTFileBufferFetcher.h
            STFileBufferFetcher.h
            XIA_CFD.h
            MemoryMap.h
        )