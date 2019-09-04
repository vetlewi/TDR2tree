// FileReader 2.0

#include "FileReader.h"
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

FileReader::FileReader(Buffer *buf_type)
    : file_stdio( nullptr )
    , errorflag( false )
    , buffer_type( buf_type )
{
}

// #########################################################

FileReader::FileReader()
    : file_stdio( nullptr )
    , errorflag( false )
    , buffer_type( new SiriusBuffer )
{
}

// #########################################################

FileReader::~FileReader()
{
    delete buffer_type;
	Close();
}

// #########################################################

bool FileReader::Open(const char *filename, const int &want)
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

    errorflag = ( file_stdio==nullptr )
                || (fseek(file_stdio, want, SEEK_SET) != want);


    return !errorflag;
}

// #########################################################

void FileReader::Close()
{
    if (file_stdio){
        std::fclose( file_stdio );
        file_stdio = nullptr;
	}
}

// #########################################################

int FileReader::Read(void *buffer, const unsigned int &size)
{
    return ReadRaw(buffer, size);
}

// #########################################################

int FileReader::Read(Buffer &buffer)
{
    return Read(buffer.GetBuffer(), buffer.GetSize());
}

// #########################################################


int FileReader::ReadRaw(void *buffer, const unsigned int &size)
{
    if ( errorflag || (!file_stdio) ){
        return -1;
    }

    int read = 0;
    while ( read < size ){
        if ( fread(reinterpret_cast<char *>(buffer)+read++, 1, 1, file_stdio) == 1 )
            continue;

        if ( feof(file_stdio) ){
            errorflag = false;
            Close();
            return read;
        } else if ( ferror(file_stdio) ) {
            errorflag = true;
            Close();
            return -1;
        }
    }
    progress.UpdateReadProgress(ftell(file_stdio));
    return read;
}

// #########################################################
