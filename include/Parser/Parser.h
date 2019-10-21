//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef PARSER_H
#define PARSER_H

#include <Parser/Entry.h>

#include <stdexcept>
#include <vector>

namespace Fetcher {
    class Buffer;
}

namespace Parser {

    class Parser {

    public:
        enum Status {
            OKAY,   //!< Entry was parsed without problems.
            END,    //!< End of buffer was reached.
            ERROR,  //!< An error while trying to parse buffer.
            WAIT    //!< A buffer might be avaliable later.
        };

        /*!
         * Return a parsed entry
         * \param status result of the parsing
         * \return An entry from the buffer
         */
        virtual std::vector<Entry_t> GetEntry(Status &status) = 0;

        //! No-op destructor
        virtual ~Parser() = default;
    };

}

#endif // PARSER_H
