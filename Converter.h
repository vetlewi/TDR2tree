//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#ifndef CONVERTER_H
#define CONVERTER_H

#include "FileReader/FileBufferFetcher.h"
#include "RootInterface/RootInterface.h"

#include "Tools/aptr.h"

class FileConverter {

private:

    //! Object storing the ROOT interface.
    RootInterface root_interface;

    //! Object storing the Buffer fetcher.
    aptr< FileBufferFetcher > bufferFetcher;

    //! Flag for addback.
    bool AddBack;

public:

    FileConverter(const char *rname, const bool &buildTree, const char *tname, const char *ttitle, bool &addback);

    bool ConvertFile(const char *fname);

};


#endif //TDR2TREE_CONVERTER_H
