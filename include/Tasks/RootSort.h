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
#include "ParticleTreeEvent.h"
#include "RFTreeEvent.h"
#include "Event.h"

namespace Task {

    class RootSort : public Base {

    private:
        ROOT::RootFileManager fileManager;
        ROOT::HistManager histManager;
        ROOT::TreeManager<TreeEvent> treeManager;
        Histogram2Dp addback_hist;

        bool tree;
        TEWordQueue_t &input_queue;

    public:

        RootSort(TEWordQueue_t &input, const char *fname, const bool &addback = true, const bool &tree = false);
        RootSort(TEWordQueue_t &input, const char *fname, const CLI::Options &options);

        void Run() override;

    };


    class RFRootSort : public Base {

        private:
            ROOT::RootFileManager fileManager;
            ROOT::HistManager histManager;
            ROOT::TreeManager<RFTreeEvent> treeManager;
            Histogram2Dp addback_hist;

            bool tree;
            TEWordQueue_t &input_queue;

        public:

            RFRootSort(TEWordQueue_t &input, const char *fname, const bool &addback = true, const bool &tree = false);
            RFRootSort(TEWordQueue_t &input, const char *fname, const CLI::Options &options);

            void Run() override;
    };

    class ParticleRootSort : public Base {
        private:
            ROOT::RootFileManager fileManager;
            ROOT::HistManager histManager;
            ROOT::TreeManager<ParticleTreeEvent> treeManager;
            Histogram2Dp addback_hist;

            bool tree;
            TEWordQueue_t &input_queue;

        public:

            ParticleRootSort(TEWordQueue_t &input, const char *fname, const bool &addback = true, const bool &tree = false);
            ParticleRootSort(TEWordQueue_t &input, const char *fname, const CLI::Options &options);

            void Run() override;
    };

}

#endif //TDR2TREE_ROOTSORT_H
