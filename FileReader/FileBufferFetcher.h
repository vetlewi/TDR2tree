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

#ifndef FILEBUFFERFETCHER_H
#define FILEBUFFERFETCHER_H

#include "BufferFetcher.h"

#include <string>
#include <memory>

class FileReader;

//! Class to read a new buffer from file.
/*!
 * \class FileBufferFetcher
 * \brief Class interface for buffer fetching classes that fetches these from file.
 * \details Interface for classes that fetches buffers from files.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class FileBufferFetcher : public BufferFetcher {
public:
	//! Open a new file.
	/*! If a file was open previously, it should be closed.
	 *
	 *	\return the status after opening the file.
	 */
    virtual Status Open(const std::string& filename,    /*!< The name of the file to open.		*/
                        int bufnum=0                    /*!< The buffer number to start from.	*/) = 0;
};

#endif // FILEBUFFERFETCHER_H
