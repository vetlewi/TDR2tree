//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#include "Buffer/STFileBufferFetcher.h"
#include "Buffer/Buffer.h"

using namespace Fetcher;

STFileBufferFetcher::STFileBufferFetcher(Buffer *buffer_template)
    : reader( new FileReader() )
    , template_buffer( buffer_template )
{}

BufferFetcher::Status STFileBufferFetcher::Open(const char *filename, size_t bufnum)
{
    int i = reader->Open( filename, bufnum*template_buffer->GetSizeChar() );
    if( i>0 ) return OKAY; else if( i==0 ) return END; else return ERROR;
}

const Buffer* STFileBufferFetcher::Next(Status& state)
{
    int i = reader->Read(reinterpret_cast<char *>(template_buffer->GetBuffer()), template_buffer->GetSizeChar() );

    if( i>0 )
        state = OKAY;
    else if( i==0 )
        state = END;
    else
        state = ERROR;

    return template_buffer.get();
}