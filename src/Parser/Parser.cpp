//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#include "Parser/Parser.h"
#include <spdlog/spdlog.h>

using namespace Parser;

Base::Base(const char *logger_name)
    : logger( spdlog::get(logger_name) ){}
