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

#include "RootFileManager.h"

#include "ProgressUI.h"

extern ProgressUI progress;

#include <vector>

#include <TTree.h>

/*!
 * TreeManager class
 * \brief The tree manager class is the interface between a built event and the ROOT tree.
 * \tparam T is an event type that implements the branches of the tree.
 */
template<class T>
class TreeManager {

private:

    TTree *tree;    //!< The tree object.
    T entry_obj;    //!< Private event object.
    bool validate;  //!< Flag to tell if the events should be validated before filling or not.

public:

    //! Constructor from RootTreeManager.
    TreeManager(RootFileManager *FileManager, const char *name, const char *title, bool validate_event = false)
        : tree( FileManager->CreateTree(name, title) )
        , entry_obj( tree, validate_event )
        , validate( validate_event ){}

    //! Add entry.
    inline void AddEntry(const T &entry)
    {
        entry_obj = entry;
        if ( validate ){
            if ( !entry_obj.IsGood() )
                return;
        }
        tree->Fill();
    }

    //! Add entries.
    void AddEntries(const std::vector<T> &entries)
    {
        progress.StartFillingTree(entries.size());
        for (int i = 0 ; i < entries.size() ; ++i){
            AddEntry(entries[i]);
            progress.UpdateTreeFillProgress(i);
        }
    }

};


#endif // TREEMANAGER_H
