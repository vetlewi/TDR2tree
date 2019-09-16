//
// Created by Vetle Wegner Ingeberg on 16/09/2019.
//

#ifndef ROOTINTERFACE_H
#define ROOTINTERFACE_H

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"

#include "Options.h"

/*
 * This class acts as the interface between the user code and ROOT.
 * Basically it will store the object that talks with the ROOT library.
 */
template<class T>
class RootInterface {

private:

    //! Structure storing the program options
    Options options;

    //! Object storing the File interface.
    RootFileManager fileManager;

    //! Object storing the histograms.
    HistManager histManager;

    //! Object storing the tree.
    TreeManager<T> treeManager;

public:

    /*!
     * Constructor
     * @param output_name - name of the output ROOT file.
     * @param buildTree - Specify if the tree should be built or not.
     * @param addback - Spesify if add-back is performed or not.
     * @param tree_name - name of the ROOT tree.
     * @param tree_title - title of the ROOT tree.
     */
    RootInterface(const Options &opt)
                  : options( opt )
                  , fileManager( options.output_file )
                  , histManager( &fileManager )
                  , treeManager( &fileManager, opt.tree_name, opt.tree_title ){}


    /*!
     * Get pointer to addback spectra
     * @return Pointer to the addback spectra.
     */
    inline TH2 *GetAB() { return histManager.GetAB(); }

    /*!
     * Fill an event.
     * @param event - structure with the events in them.
     */
    inline void Fill(const T &event){
        histManager.AddEntry(event);
        if ( options.buildTree )
            treeManager.AddEntry(event);
    }

};


#endif // ROOTINTERFACE_H
