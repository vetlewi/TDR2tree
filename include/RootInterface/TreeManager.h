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

#include <Buffer/aptr.h>
#include "RootFileManager.h"
#include "Event/Event.h"

#include <TTree.h>

#include <mutex>

class TreeManager
{
protected:

    TTree *tree;            //!< Pointer to the tree to write events to.
    Event::Base *entry_obj; //!< Object to fill (set at run time

public:

    TreeManager(RootFileManager *fm, const char *name, const char *title, Event::Base *type)
        : tree( fm->CreateTree(name, title) )
        , entry_obj( type )
        {
            entry_obj->SetupTree(tree);
        }

#if ROOT_MT_FLAG
    TreeManager(RootMergeFileManager *fm, const char *name, const char *title, Event::Base *type)
            : tree( fm->CreateTree(name, title) )
            , entry_obj( type )
    {
        entry_obj->SetupTree(tree);
    }
#endif // ROOT_MT_FLAG

    ~TreeManager()
    {
        delete entry_obj;
    }

    inline void AddEntry(Event::Base *entry)
    {
        entry_obj->Copy(entry);
        tree->Fill();
    }
};

/*!
 * TreeManager class
 * \brief The tree manager class is the interface between a built event and the ROOT tree.
 * \tparam T is an event type that implements the branches of the tree.
 */
template<class T>
class TreeMangr
{
private:

    TTree *tree;
    T entry_obj;



public:

    TreeMangr(RootFileManager *fm, const char *name, const char *title)
        : tree( fm->CreateTree(name, title) )
        , entry_obj( tree ){}

    //! Add an entry
    inline void AddEntry(const Event::Base *entry)
    {
        entry_obj.Copy(reinterpret_cast<const T *>(entry));
        tree->Fill();
    }

};


#endif // TREEMANAGER_H
