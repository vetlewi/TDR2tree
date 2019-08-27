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

/*!
 * \file STFileBufferFetcher.cpp
 * \brief Implementation of STFileBufferFetcher.
 * \author Vetle W. Ingeberg
 * \date 2015-2016
 * \copyright GNU Public License v. 3
 */

#include "STFileBufferFetcher.h"

const TDRBuffer* STFileBufferFetcher::Next(Status& state)
{
    int i = reader.Read( &buffer );
    if ( i == buffer.GetSize() ){
        state = OKAY;
    } else if ( i > 0 ){
        state = END;
        word_t *tmp = new word_t[i];
        for (int num = 0 ; num < i ; ++num){
            tmp[num] = buffer[i];
        }
        buffer.Resize(i);
        for (int num = 0 ; num < i ; ++num){
            buffer[i] = tmp[i];
        }
        delete[] tmp;
    } else {
        state = ERROR;
    }
	return &buffer;
}
