//
// Created by Vetle Wegner Ingeberg on 16/04/2021.
//

#ifndef TDR2TREE_TRIGGER_H
#define TDR2TREE_TRIGGER_H

#include "Task.h"
#include "experimentsetup.h"

namespace Task {

    class Trigger : public Base
    {
    private:
        MCWordQueue_t &input_queue;
        TEWordQueue_t output_queue;

        const double coincidence_time;
        const DetectorType trigger;

    public:
        Trigger(MCWordQueue_t &input, const double &time = 1500., const DetectorType &trigger = DetectorType::eDet, const size_t &cap = 65536);
        TEWordQueue_t &GetQueue(){ return output_queue; }

        void Run() override;

    };

}

#endif //TDR2TREE_TRIGGER_H
