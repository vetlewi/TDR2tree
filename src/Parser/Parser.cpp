//
// Created by Vetle Wegner Ingeberg on 12/11/2019.
//

#include "Parser/Parser.h"

#if LOG_ENABLED
#include <spdlog/spdlog.h>
#endif // LOG_ENABLED

using namespace Parser;

#if LOG_ENABLED
Base::Base(const char *logger_name)
    : logger( spdlog::get(logger_name) ){}
#else
Base::Base(const char *){}
#endif // LOG_ENABLED