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

#ifndef FILEREADER_H
#define FILEREADER_H

#include <cstdio>
#include <cstdint>
#include <vector>

#include "BasicStruct.h"

class TDRBuffer;

/*!
 * \class FileReader
 * \brief Class for reading TDR buffers from file.
 * \details This class reads the TDR buffers from binary files. It removes all CFD values from the stream. It also decodes the binary format to
 * the WordBuffer type.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class FileReader {

public:

	//! Initilizer
	FileReader();

	//! Destructor
	~FileReader();

	//! Open a file.
	/*! \return true if opening was successful.
	 */
    bool Open(const char *filename, /*!< Name of the file to open.	*/
              int seekpos=0         /*!< Where to open the file at.	*/);

	//! Read a single buffer from the file.
    /*! \return 1 for new buffer, 0 if EOF is reached or -1 if
     *  an error was encountred.
	 */
    int Read(word_t *buffer,    /*!< Buffer to put the data. 	*/
             int size           /*!< How many hits to read.     */);

    int Read(TDRBuffer *buffer);

    //! Read a

    //! Check the error flag.
    /*! \return The error flag.
     */
    bool IsError() const
    	{ return errorflag; }

    //! Read entire file to memory.
    static std::vector<word_t> GetFile(const char *fname);

private:
	//! The object for reading files.
    std::FILE * file_stdio;

	//! Close the file.
    void Close();

	//! Number of buffers that have been read.
	bool errorflag;

    //! Method for reading and parsing an event from the file.
    bool ReadEvent(word_t &hit);
};

#endif // FILEREADER_H
