//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#include "Converter.h"

#include "FileReader/TDRFileReader.h"
#include "FileReader/MTFileBufferFetcher.h"

Event::TDREntry template_event;

FileConverter::FileConverter(const char *rname, const bool &buildTree, const char *tname, const char *ttitle, bool &addback)
    : root_interface(rname, reinterpret_cast<Event::Base *>(&template_event), buildTree, tname, ttitle )
    , bufferFetcher( new MTFileBufferFetcher(new TDRFileReader) )
{

}