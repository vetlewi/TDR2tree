// FileReader 2.0

#include "BasicFileReader.h"
#include "experimentsetup.h"
#include "Calibration.h"
#include "XIA_CFD.h"
#include "ProgressUI.h"

#include "BufferType.h"

#include <cstdint>
#include <algorithm>
#include <iostream>

extern ProgressUI progress;

// #########################################################

BasicFileReader::BasicFileReader()
    : file_stdio( nullptr )
    , errorflag( false )
{
}

// #########################################################

BasicFileReader::~BasicFileReader()
{
    Close();
}

// #########################################################

bool BasicFileReader::Open(const char *filename, int want)
{
    Close();
    file_stdio = std::fopen(filename, "rb");

    if ( !file_stdio ){
        std::cerr << __PRETTY_FUNCTION__ << ": Unable to open file '" << filename << "'" << std::endl;
        exit(EXIT_FAILURE);
    }

    fseek(file_stdio, 0, SEEK_END);
    auto size = size_t(ftell(file_stdio));
    fseek(file_stdio, 0, SEEK_SET);
    progress.StartNewFile(filename, size);


    errorflag = ( file_stdio == nullptr )
                || ( fseek(file_stdio, want, SEEK_SET) );


    return !errorflag;
}

// #########################################################

void BasicFileReader::Close()
{
    if (file_stdio){
        std::fclose( file_stdio );
        file_stdio = nullptr;
    }
}

// #########################################################

int BasicFileReader::Read(TDRByteBuffer *buffer)
{

    if ( errorflag || (!file_stdio) ){
        return -1;
    }

    size_t have = 0;
    while ( have < buffer->GetSize() ){
        if ( std::fread(buffer->GetBuffer() + have, sizeof(char), 1, file_stdio) != 1 ){
            if ( feof( file_stdio ) ){
                Close();
                // We will need to do some rebuilding of stuff...
                char *tmp = new char[buffer->GetSize()];
                memcpy(tmp, buffer->GetBuffer(), 65536);
                buffer->Resize(have);
                memcpy(buffer->GetBuffer(), tmp, have);
                delete[] tmp;
            } else {
                errorflag = true;
                Close();
            }
            return errorflag ? -1 : 0;
        } else {
            ++have;
        }
    }
    progress.UpdateReadProgress(ftell(file_stdio));
    return 1;
}