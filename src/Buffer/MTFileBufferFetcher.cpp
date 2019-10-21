/*
 * MTFileBufferFetcher.cpp
 *
 *  Created on: 10.03.2010
 *      Author: Alexander Bürger
 */

#include "Buffer/MTFileBufferFetcher.h"
#include "Buffer/aptr.h"
#include "Buffer/FileReader.h"
#include "Buffer/Buffer.h"

#include "PrefetchThread.h"

using namespace Fetcher;

#include <cstdlib>
#include <iostream>
#include <pthread.h>

MTFileBufferFetcher::MTFileBufferFetcher(Buffer *buffer_template)
        : reader( new FileReader() )
        , template_buffer( buffer_template )
        , prefetch( nullptr )
{
}

// ########################################################################

MTFileBufferFetcher::~MTFileBufferFetcher()
{
    StopPrefetching();
}

// ########################################################################

const Buffer* MTFileBufferFetcher::Next(Status& state)
{
    if( reader->IsError() ) {
        state = ERROR;
        return nullptr;
    }

    if( !prefetch ) {
        prefetch = new Details::PrefetchThread( reader.get(), template_buffer.get() );
        prefetch->Start();
    } else {
        // finish reading the buffer from the last call to Next()
        prefetch->ReadingEnds();
    }

    // fetch the next buffer
    const Buffer* b = prefetch->ReadingBegins();
    state = b ? OKAY : END;
    return b;
}

// ########################################################################

BufferFetcher::Status MTFileBufferFetcher::Open(const char *filename, size_t bufnum)
{
    StopPrefetching();
    int i = reader->Open( filename, bufnum*template_buffer->GetSizeChar() );
    if( i>0 ) return OKAY; else if( i==0 ) return END; else return ERROR;
}

// ########################################################################

void MTFileBufferFetcher::StopPrefetching()
{
    if( !prefetch )
        return;

    prefetch->Stop();
    delete prefetch;
    prefetch = nullptr;
}