//
// Created by Vetle Wegner Ingeberg on 19/04/2021.
//

#ifndef TDR2TREE_ROOTSORT_H
#define TDR2TREE_ROOTSORT_H

#include "Task.h"
#include "CommandLineInterface.h"

#include "RootFileManager.h"
#include "HistManager.h"
#include "TreeManager.h"
#include "TreeEvent.h"

namespace Task {

    class RootSort : public Base {

    private:
        ROOT::RootFileManager fileManager;
        ROOT::HistManager histManager;
        ROOT::TreeManager<TreeEvent> treeManager;
        Histogram2Dp addback_hist;

        bool tree;
        MCWordQueue_t &input_queue;

    public:

        RootSort(MCWordQueue_t &input, const char *fname, const bool &addback = true, const bool &tree = false);
        RootSort(MCWordQueue_t &input, const char *fname, const CLI::Options &options);
        void Run() override;

    };

}

#endif //TDR2TREE_ROOTSORT_H
