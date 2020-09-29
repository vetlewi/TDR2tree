//
// Created by Vetle Wegner Ingeberg on 23/03/2020.
//

#ifndef XIAPARSER_H
#define XIAPARSER_H

#include "Entry.h"
#include "Parser.h"

#include <vector>

#include <cstdint>

namespace Parser {

    class XIAparser : public Base {

    public:

        /*!
         * Initialize everything to zero
         */
        explicit XIAparser(const char *logger = "logger") : Base(logger){}

        /*!
         * Get next entry
         * \param status
         * \return
         */
        std::vector<Entry_t> GetEntry(const Fetcher::Buffer *new_buffer) override;

    private:

        //! A buffer in cases where an event is split across two actual buffers
        std::vector<uint32_t> spill_buffer;




    };

}


#endif // XIAPARSER_H
