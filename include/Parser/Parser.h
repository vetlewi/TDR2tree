//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef PARSER_H
#define PARSER_H

#include "Parser/Entry.h"

#include <stdexcept>
#include <vector>


namespace spdlog {
    class logger;
}

namespace Fetcher {
    class Buffer;
}

namespace Parser {

    class Base {

    public:
        Base() = default;

        explicit Base(const char *logger_name = "logger"){}

        /*!
         * Return a parsed entry
         * \param status result of the parsing
         * \return An entry from the buffer
         */
        virtual std::vector<Entry_t> GetEntry(const Fetcher::Buffer *buffer) = 0;

        //! No-op destructor
        virtual ~Base() = default;

        /*!
         * Overload of the () operator.
         * \param buffer see GetEntry()
         * \return GetEntry()
         */
        inline std::vector<Entry_t> operator()(const Fetcher::Buffer *buffer){ return GetEntry(buffer); }

    protected:

        //std::shared_ptr<spdlog::logger> logger;

    };

}

#endif // PARSER_H
