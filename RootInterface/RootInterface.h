//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#ifndef ROOTINTERFACE_H
#define ROOTINTERFACE_H

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"

#include "Event.h"
#include "TDREvent.h"

/*
 * This class acts as the interface between the user code and ROOT.
 * Basically it will store the object that talks with the ROOT library.
 */
class RootInterface {

private:

    //! Object storing the File interface.
    RootFileManager fileManager;

    //! Object storing the histograms.
    HistManager histManager;

    //! Object storing the tree.
    TreeManager treeManager;

    //! Flag indicating if trees are to be built or not.
    bool buildTree;

public:

    /*!
     * Constructor
     * @param output_name - name of the output ROOT file.
     * @param buildTree - Specify if the tree should be built or not.
     * @param addback - Spesify if add-back is performed or not.
     * @param tree_name - name of the ROOT tree.
     * @param tree_title - title of the ROOT tree.
     */
    RootInterface(const char *rname, Event::Base *template_event, const bool BuildTree=false, const char *tname=nullptr, const char *ttitle=nullptr)
        : fileManager( rname )
        , histManager( &fileManager )
        , treeManager( &fileManager, tname, ttitle, template_event )
        , buildTree( BuildTree ){}


    /*!
     * Get pointer to addback spectra
     * @return Pointer to the addback spectra.
     */
    inline TH2 *GetAB() { return histManager.GetAB(); }

    /*!
     * Fill an event.
     * @param event - structure with the events in them.
     */
    inline void Fill(Event::TDREvent &event){
        histManager.AddEntry(event);
        if ( buildTree )
            treeManager.AddEntry(&event);
    }

};


#endif // ROOTINTERFACE_H
