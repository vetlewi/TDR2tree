/* -*- c++ -*-
 * STFileBufferFetcher.h
 *
 *  Created on: 10.03.2010
 *      Author: Alexander Bürger
 */

#ifndef STFILEBUFFERFETCHER_H
#define STFILEBUFFERFETCHER_H

#include <stdexcept>

#include "FileBufferFetcher.h"
#include "FileReader.h"
#include "aptr.h"

namespace Fetcher {

//! Class for fetching buffers from a file.
/*! This class does not prefetch buffers or spawn a reader thread.
 */
    class STFileBufferFetcher : public FileBufferFetcher
    {

    public:

        //! Initialize with correct buffer type
        explicit STFileBufferFetcher(Buffer *template_buffer) : buffer( template_buffer )
        {
            if ( !buffer )
                throw std::runtime_error("No template buffer was provided.");
        }

        /*! Calls the reader to open a file. */
        Status Open(const char *filename, size_t bufnum) override;


        /*! Calls the reader to fetch a buffer. */
        const Buffer *Next(Status &state) override;

    private:
        //! The class implementing the actual reading.
        FileReader reader;

        //! The buffer used to store the file data in.
        Buffer *buffer;
    };

}

#endif /* STFILEBUFFERFETCHER_H */