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


#ifndef MTFILEBUFFERFETCHER_H
#define MTFILEBUFFERFETCHER_H

#include "aptr.h"
#include "FileBufferFetcher.h"

#include <string>

class FileReader;
class TDRBuffer;

//! Fetch buffers from a file in a separate thread.
/*! This class spawns a thread to prefetch buffers on the
 *  fist call to Next().
 */
/*!
 * \class MTFileBufferFetcher
 * \brief Fetch buffers from a file in a separate thread.
 * \author Vetle W. Ingeberg
 * \author Alexander Bürger
 * \date 2010-2016
 * \copyright GNU Public License v. 3
 * \todo The performance may be increased by using more than one separate thread.
 */
class MTFileBufferFetcher : public FileBufferFetcher {
public:
	//! Construct the buffer fetcher.
	MTFileBufferFetcher();

	//! Closes the file, if still open.
	~MTFileBufferFetcher();

    //! Open a new file.
    /*! \return the result of the opening of the file.
     */
    Status Open(const std::string& filename,    /*!< Filename of the file to read.          */
                int bufnum = 0                  /*!< What buffer no. to start reading from. */);

	/*! Creates a new thread which prefetches some buffers while
	 * the main thread is sorting.
	 */
	const TDRBuffer* Next(Status& state);

	//! Set the buffer template.
    void SetBuffer(TDRBuffer *buf)
		{ template_buffer.reset( buf ); }

private:
	//! Stop the prefetch thread.
	void StopPrefetching();

    //! Class performing the reading.
	aptr<FileReader> reader;

    //! Template for the buffer.
	aptr<TDRBuffer> template_buffer;

    //! Class to prefetch a buffer.
	class PrefetchThread* prefetch;
};

#endif // MTFILEBUFFERFETCHER_H
