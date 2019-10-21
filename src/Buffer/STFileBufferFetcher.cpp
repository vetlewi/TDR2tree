//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include "Buffer/STFileBufferFetcher.h"
#include "Buffer/Buffer.h"

using namespace Fetcher;

BufferFetcher::Status STFileBufferFetcher::Open(const char *filename, size_t bufnum)
{
    return reader.Open(filename, bufnum * buffer->GetSizeChar()) ? OKAY : ERROR;
}

const Buffer* STFileBufferFetcher::Next(Status& state)
{
    int i = reader.Read(reinterpret_cast<char *>(buffer->GetBuffer()), buffer->GetSizeChar() );

    if( i>0 )
        state = OKAY;
    else if( i==0 )
        state = END;
    else
        state = ERROR;

    return buffer;
}