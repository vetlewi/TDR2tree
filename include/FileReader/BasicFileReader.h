//
// Created by Vetle Wegner Ingeberg on 11/03/2021.
//

#ifndef TDR2TREE_BASICFILEREADER_H
#define TDR2TREE_BASICFILEREADER_H

#include <cstdio>
#include <cstdint>
#include <vector>

#include "BasicStruct.h"

class TDRByteBuffer;

/*!
 * \class BasicFileReader
 * \brief Class for reading bytes file.
 * \details This class reads the TDR buffers from binary files. It removes all CFD values from the stream. It also decodes the binary format to
 * the WordBuffer type.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */
class BasicFileReader {

public:

    //! Initilizer
    BasicFileReader();

    //! Destructor
    ~BasicFileReader();

    //! Open a file.
    /*! \return true if opening was successful.
     */
    bool Open(const char *filename, /*!< Name of the file to open.	*/
              int seekpos=0         /*!< Where to open the file at.	*/);

    //! Read a single buffer from the file.
    /*! \return 1 for new buffer, 0 if EOF is reached or -1 if
     *  an error was encountred.
	 */
    int Read(TDRByteBuffer *buffer);


    //! Check the error flag.
    /*! \return The error flag.
     */
    bool IsError() const
    { return errorflag; }

private:
    //! The object for reading files.
    std::FILE * file_stdio;

    //! Close the file.
    void Close();

    //! Number of buffers that have been read.
    bool errorflag;
};


#endif //TDR2TREE_BASICFILEREADER_H
