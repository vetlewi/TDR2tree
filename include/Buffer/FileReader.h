/* -*- c++ -*-
* FileReader.h
*
*  Modified on: 05.09.2011
*      Author: Alexander Bürger
*/

#ifndef FILEREADER_H
#define FILEREADER_H

#include <cstdio>
#if HAVE_ZLIB
    #include <zlib.h>
#endif

namespace Fetcher {



    class Buffer;

    //! Class for reading buffers from a file.
    /*! This class performs the actual reading for both
     *  STFileBufferFetcher and MTFileBufferFetcher.
     *
     * It can be compiled to read both files compressed with gzip
     * (filename ending with <code>.gz</code>) and not compressed files
     * (any other ending).
     */
    class FileReader {
    public:
        FileReader();

        //! Close file if still open.
        ~FileReader();

        //! Open a file and go to the specified position.
        /*! \return true if both opening and seeking were successful.
         */

        /*!
         * Open a file and go to the specified position
         * \param filename Path to file to open
         * \param seekpos At which byte to start reading from
         * \return true if both opening and seeking were successful, false otherwise
         */
        bool Open(const char *filename, size_t seekpos);

        /*!
         * Read a buffer from the file
         * \param data The buffer to store the data from file into
         * \param size How many bytes to read
         * \return 1 for new buffer, 0 for end of file, -1 for error.
         */
        int Read(char* data, size_t size);

        //! Retrieve error flag.
        /*! \return The error flag.
         */
        bool IsError() const { return errorflag; }

    private:

        //! Close the file, reset the error flag.
        void Close();

        //! The object for reading uncompressed files.
        std::FILE* file_stdio;

#if HAVE_ZLIB
        //! The object for reading gzip'ed files.
        gzFile file_gz;
#endif

        //! The error flag.
        bool errorflag;
    };
}


#endif // FILEREADER_H
