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

#ifndef BUFFERFETCHER_H
#define BUFFERFETCHER_H

class TDRBuffer;

//! Class to fetch a new buffer.
/*!
 * \class BufferFetcher
 * \brief Interface for buffer fetcher classes.
 * \details This class defines the interface for classes used for fetching buffers.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class BufferFetcher {
public:
	typedef enum {
        OKAY,		//!< Buffer was fetched without problems.
		END,		//!< End of buffer stream was reached.
		ERROR,		//!< An error while trying to fetch buffer.
		WAIT		//!< A buffer might be avalible later.
	} Status;

	//! Fetch the next buffer.
	/*! \return OKEY if buffer was fetched, END if there
	 *  are no more buffers, ERROR in case of error, WAIT if
	 *  if fetching a buffer might be possible later.
	 */
    virtual const TDRBuffer* Next(Status& state /*!< Will contain the status after reading. */) = 0;

    //! Destructor.
    virtual ~BufferFetcher() { }
};

#endif // BUFFERFETCHER_H
