/*******************************************************************************
 * Copyright (C) 2019 Vetle W. Ingeberg                                        *
 * Author: Vetle Wegner Ingeberg, vetlewi@fys.uio.no                           *
 *                                                                             *
 * --------------------------------------------------------------------------- *
 * This program is free software; you can redistribute it and/or modify it     *
 * under the terms of the GNU General Public License as published by the       *
 * Free Software Foundation; either version 3 of the license, or (at your      *
 * option) any later version.                                                  *
 *                                                                             *
 * This program is distributed in the hope that it will be useful, but         *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                  *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General   *
 * Public License for more details.                                            *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#ifndef EVENT_H
#define EVENT_H

#include <stdint.h>
#include <vector>

class TTree;
class word_t;

#define NUM_MAX 64

struct EventEntry
{
    int16_t ID;         //!< ID of the detector of this event.
    double energy;      //!< Energy of the event [keV].
    double tfine;       //!< Correction to the timestamp [ns].
    int64_t tcoarse;    //!< Timestamp [ns].
};

class Event
{

private:

    int ring_mult;                      //!< Ring multiplicity.
    EventEntry ringEvent[NUM_MAX];      //!< Ring event structures.

    int sect_mult;                      //!< Sector multiplicity.
    EventEntry sectEvent[NUM_MAX];      //!< Sector event structures.

    int back_mult;                      //!< Back multiplicity.
    EventEntry backEvent[NUM_MAX];      //!< Back event structures.

    int labrL_mult;                     //!< Large volume LaBr multiplicity.
    EventEntry labrLEvent[NUM_MAX];     //!< Large volume LaBr event structures.

    int labrS_mult;                     //!< Slow FTA LaBr multiplicity.
    EventEntry labrSEvent[NUM_MAX];     //!< Slow FTA LaBr event structures.

    int labrF_mult;                     //!< Fast FTA LaBr multiplicity.
    EventEntry labrFEvent[NUM_MAX];     //!< Fast FTA LaBr event structures.

    int clover_mult;                    //!< Clover multiplicity.
    EventEntry cloverEvent[NUM_MAX];    //!< Clover event structures.

    int rfMult;                         //!< RF signal multiplicity.
    EventEntry rfEvent[NUM_MAX];        //!< RF signal event structures.


public:

    //! Constructor.
    /*!
     * This constructor is meant to use for the TreeManager. This will set the branches of the tree.
     * \param tree ROOT TTree to attach branches to.
     */
    explicit Event(TTree *tree);

    //! Constructor.
    explicit Event(const std::vector<const word_t *> &event, const word_t &trigger);

    //! Assignment operator.
    Event &operator=(const Event &event);

};


#endif // EVENT_H
