/*******************************************************************************
 * Copyright (C) 2016 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, v.w.ingeberg@fys.uio.no                      *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#ifndef STFILEBUFFERFETCHER_H
#define STFILEBUFFERFETCHER_H

#include "FileBufferFetcher.h"
#include "FileReader.h"

#include "aptr.h"

#include <string>


//! Class for fetching buffers from a file.
/*! This class does not prefetch buffers or spawn a reader thread.
 */

/*!
 * \class STFileBufferFetcher
 * \brief Buffer fetcher class that fetches the buffers in the same thread as the main program.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class STFileBufferFetcher : public FileBufferFetcher {
public:

    //! Construct with correct FileReader and Buffer type.
    explicit STFileBufferFetcher(FileReader *freader) : FileBufferFetcher( freader ), buffer( template_buffer->New() ){}

    ~STFileBufferFetcher() override { delete buffer; }

	//! Calls the reader to open a file.
    Status Open(const char *filename,    /*!< File to read.          */
                int bufnum               /*!< First buffer to read.  */) override
		{return reader->Open(filename, bufnum) ? OKAY : ERROR; }

	//! Calls the reader to fetch a buffer.
    /*! \return Pointer to the buffer that have been read.
     */
    const Buffer* Next(Status& state    /*!< Result of the reading process. */) override;

private:

    //! Actual object that are filling.
    Buffer *buffer;
};

#endif // STFILEBUFFERFETCHER_H
