//
// Created by Vetle Wegner Ingeberg on 2019-05-28.
//

#ifndef TDR2TREE_PROGRESSUI_H
#define TDR2TREE_PROGRESSUI_H

#include <string>

#define BARWIDTH 50

class ProgressUI {

private:
    std::string curr_fname; //!< Name of current file being processed.
    size_t length;
    int shown;

public:

    //! Default ctor.
    ProgressUI() : curr_fname( "" ), length( 0 ), shown( 0 ) {}

    //! Give the user feedback that we are starting the readout of a new file.
    void StartNewFile(const std::string &fname, const size_t &flength);

    //! Update read progress of the file.
    void UpdateReadProgress(const size_t &curr_pos);

    //! Give the user feedback that we will start building events.
    void StartBuildingEvents(const size_t &length);

    //! Update event building progress.
    void UpdateEventBuildingProgress(const size_t &curr_pos);

    //! Give the user feedback that we are filling histograms.
    void StartFillingHistograms(const size_t &length);

    //! Update the histogram filling progress.
    void UpdateHistFillProgress(const size_t &curr_pos);

    //! Give the user feedback that we are filling ROOT tree.
    void StartFillingTree(const size_t &length);

    //! Update the tree filling status.
    void UpdateTreeFillProgress(const size_t &curr_pos);

    //! Notify the user that we are done sorting the file.
    void Finish();
};


#endif //TDR2TREE_PROGRESSUI_H
