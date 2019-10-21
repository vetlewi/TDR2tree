//
// Created by Vetle Wegner Ingeberg on 17/10/2019.
//

#ifndef TDR2TREE_TDRPARSER_H
#define TDR2TREE_TDRPARSER_H

#include "Parser.h"
#include "TDRtypes.h"

namespace Parser {

    class TDRparser : public Parser {

    public:

        /*!
         * Initialize everything to zero
         */
        TDRparser() : top_time( -1 ){}

        /*!
         * Get next entry
         * \param status
         * \return
         */
        std::vector<Entry_t> GetEntry(const Fetcher::Buffer *new_buffer, Status &status);

    private:

        //! Top 32-bit of the timestamp
        int64_t top_time;

        std::vector<TDR_leftover_entries> leftover_entries;

        std::vector<Entry_t> SortMerge(std::vector<TDR_entry> &entries);

    };

}

#endif //TDR2TREE_TDRPARSER_H
