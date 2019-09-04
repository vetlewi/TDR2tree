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

#include "BufferType.h"

typedef struct __sFILE FILE;
class Buffer;

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

protected:

    //! Private initializer used by derived classes.
    FileReader(Buffer *buf_type);

public:

	//! Initilizer
	FileReader();

	//! Destructor
	~FileReader();

	//! Open a file.
	/*! \return true if opening was successful.
	 */
    bool Open(const char *filename,    /*!< Name of the file to open.	*/
              const int &seekpos=0    /*!< Where to open the file at.	*/);

	//! Read data from file.
    /*!
     * \return size if entire buffer was read, the amount of data read if EOF and -1 if error.
	 */
    virtual int Read(void *buffer,                /*!< Buffer to put the data.  */
                     const unsigned int &size     /*!< How many hits to read.   */);

    //! Read data from file.
    /*!
    * \return 1 if successful, 0 if EOF and -1 if an error was encountred.
    */
    virtual int Read(Buffer &buffer);


    //! Check the error flag.
    /*! \return The error flag.
     */
    bool IsError() const
    	{ return errorflag; }

    //! Check if file is open or not.
    inline bool IsOpen() const { return ( file_stdio != nullptr ); }

    //! Get a pointer to the buffer type.
    inline Buffer *GetBufferType() const { return buffer_type; }

protected:

    //! Read raw data from file (used by derived classes).
    /*!
     * Read raw data from file.
     * \param buffer
     * \param size
     * \return size if successful, actual amount of data read if EOF and -1 if error.
     */
    int ReadRaw(void *buffer,               /*!< Pointer to memory where the raw data should be read to */
                const unsigned int &size    /*!< Amount of data to read in bytes.                       */);

private:
	//! The object for reading files.
    FILE * file_stdio;

    //! Number of buffers that have been read.
    bool errorflag;

    //! A static object that defines the buffer type.
    Buffer *buffer_type;

	//! Close the file.
    void Close();

};

#endif // FILEREADER_H
