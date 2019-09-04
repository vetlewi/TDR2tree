//
// Created by Vetle Wegner Ingeberg on 02/09/2019.
//

#include "BasicStruct.h"

bool operator<(const word_t &lhs, const word_t &rhs)
{
    return double( lhs.timestamp - rhs.timestamp ) + ( lhs.cfdcorr - rhs.cfdcorr ) < 0;
}