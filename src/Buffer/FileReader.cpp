/*
 * FileReader.cpp
 *
 *  Modified on: 05.09.2011
 *      Author: Alexander Bürger
 */

#include "Buffer/FileReader.h"

#include <string>

using namespace Fetcher;

FileReader::FileReader()
        : file_stdio( nullptr )
#if HAVE_ZLIB
        , file_gz(0)
#endif
        , errorflag( true )
{
}

// ########################################################################

FileReader::~FileReader()
{
    Close();
}

// ########################################################################

int FileReader::Read(char* data, size_t size_req)
{
#if HAVE_ZLIB
    if( errorflag || (!file_stdio && !file_gz) )
#else
     if( errorflag || !file_stdio )
#endif
    {
        return -1;
    }

    unsigned int have = 0;
    while( have<size_req ) {
        int now = -1;
        if( file_stdio )
            now = std::fread(data+have, 1, size_req-have, file_stdio);
#if HAVE_ZLIB
        else if( file_gz )
            now = gzread(file_gz, data+have, size_req-have);
#endif
        if( now<=0 ) {
            errorflag = (now<0 || (have!= 0 && have!=size_req));
            Close();
            return errorflag ? -1 : 0;
        } else {
            have += now;
        }
    }
    return 1;
}

// ########################################################################

bool FileReader::Open(const char *filename, size_t want)
{
    Close();
    std::string fname = filename;
    if( fname.find(".gz") == fname.size()-3 ) {
#if HAVE_ZLIB
        file_gz = gzopen(filename, "rb");
        errorflag = (file_gz == 0)
                    || gzseek(file_gz, want, SEEK_SET) != want;
#else
        throw std::runtime_error("FileReader::Open(): zlib missing");
#endif
    } else {
        file_stdio = fopen(filename, "rb");
        errorflag = (file_stdio == 0)
                    || std::fseek(file_stdio, want, SEEK_SET) != want;
    }
    return !errorflag;
}

// ########################################################################

void FileReader::Close()
{
    if( file_stdio ) {
        std::fclose(file_stdio);
        file_stdio = 0;
    }
#if HAVE_ZLIB
    if( file_gz ) {
        gzclose(file_gz);
        file_gz = 0;
    }
#endif
}