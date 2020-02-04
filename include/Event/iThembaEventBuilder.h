//
// Created by Vetle Wegner Ingeberg on 31/10/2019.
//

#ifndef ITHEMBAEVENTBUILDER_H
#define ITHEMBAEVENTBUILDER_H

#include <vector>

#include "Event/iThembaEvent.h"

#include <Parser/Entry.h>

namespace Event {

    class iThembaEventBuilder {

    public:

        std::vector<iThembaEvent> BuildEvents(std::vector<Parser::Entry_t> &entries);

        std::vector<iThembaEvent> FlushEvents();

    private:

        std::vector<Parser::Entry_t> entry_buffer;
    };

}


#endif // ITHEMBAEVENTBUILDER_H
