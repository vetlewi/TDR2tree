//
// Created by Vetle Wegner Ingeberg on 23/03/2020.
//

#include "Parser/XIAparser.h"

using namespace Parser;

struct XIA_header {

};



std::vector<Entry_t> XIAparser::GetEntry(const Fetcher::Buffer *new_buffer)
{
    std::vector<Entry_t> found;
    int read = 0; // Current read position
    // First check if there is a spillover...
    if ( !spill_buffer.empty() ){


    }

}