/*
 * FileReader.cpp
 *
 *  Modified on: 05.09.2011
 *      Author: Alexander Bürger
 */

#include "Buffer/FileReader.h"

#include <string>

#include <Utilities/ProgressUI.h>

extern ProgressUI progress;

using namespace Fetcher;

FileReader::FileReader()
        : file_stdio( nullptr )
#if HAVE_ZLIB
        , file_gz( nullptr )
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
            progress.Finish();
            return errorflag ? -1 : 0;
        } else {
            have += now;
        }
    }
    if ( file_stdio )
        progress.UpdateReadProgress(ftell(file_stdio));
    else if ( file_gz )
        progress.UpdateReadProgress(gztell(file_gz));
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

        gzseek(file_gz, 0, SEEK_END);
        progress.StartNewFile(filename, gztell(file_gz));
        gzseek(file_gz, 0, SEEK_SET);
        errorflag = (file_gz == nullptr)
                    || gzseek(file_gz, want, SEEK_SET) != want;
#else
        throw std::runtime_error("FileReader::Open(): zlib missing");
#endif
    } else {
        file_stdio = fopen(filename, "rb");

        fseek(file_stdio, 0, SEEK_END);
        progress.StartNewFile(filename, ftell(file_stdio));
        fseek(file_stdio, 0, SEEK_SET);
        errorflag = (file_stdio == nullptr)
                    || std::fseek(file_stdio, want, SEEK_SET) != want;
    }
    return !errorflag;
}

// ########################################################################

void FileReader::Close()
{
    if( file_stdio ) {
        std::fclose(file_stdio);
        file_stdio = nullptr;
    }
#if HAVE_ZLIB
    if( file_gz ) {
        gzclose(file_gz);
        file_gz = nullptr;
    }
#endif
}