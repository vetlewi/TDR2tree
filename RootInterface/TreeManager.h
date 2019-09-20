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
 * You should have recived a copy of the GNU General Public License along with *
 * the program. If not, see <http://www.gnu.org/licenses/>.                    *
 *                                                                             *
 *******************************************************************************/

#ifndef TREEMANAGER_H
#define TREEMANAGER_H

#include "aptr.h"
#include "RootFileManager.h"
#include "Event.h"

#include <TTree.h>


/*!
 * TreeManager class
 * \brief The tree manager class is the interface between a built event and the ROOT tree.
 * \tparam T is an event type that implements the branches of the tree.
 */
class TreeManager {

private:

    TTree *tree;    //!< The tree object.
    aptr<Event::Base> entry_obj;    //!< Private event object.

public:

    //! Constructor from RootTreeManager.
    TreeManager(RootFileManager *FileManager, const char *name, const char *title, Event::Base *template_event );

    //! Add entry.
    void AddEntry(Event::Base *entry)
    {
        entry_obj->Copy(entry);
        tree->Fill();
    }

};


#endif // TREEMANAGER_H
