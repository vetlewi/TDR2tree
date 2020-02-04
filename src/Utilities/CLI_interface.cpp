//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#include "Utilities/CLI_interface.h"

#include <Parser/Parser.h>

// Clean-up
Settings_t::~Settings_t()
{
    delete buffer_type;
    delete parser;
    delete event_type;

    delete input_queue;
    delete split_queue;
    delete built_queue;
    delete str_queue;
}