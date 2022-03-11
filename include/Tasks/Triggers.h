//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#ifndef TDR2TREE_TRIGGERS_H
#define TDR2TREE_TRIGGERS_H

#include <Task.h>

namespace Task {

    class Trigger_worker : public Base
    {
    private:
        MCWordQueue_t &input_queue;
        TEWordQueue_t &output_queue;

        const double coincidence_time;
        const DetectorType trigger;

    public:
        Trigger_worker(MCWordQueue_t &input, TEWordQueue_t &output, const double &time = 1500., const DetectorType &trigger = DetectorType::eDet);
        void Run() override;

    };

    class Triggers
    {
    private:
        MCWordQueue_t &input_queue;
        TEWordQueue_t output_queue;
        std::vector<Trigger_worker *> triggers;
    public:
        Triggers(MCWordQueue_t &input, const size_t &workers = 4, const double &time = 1500., const DetectorType &trigger = DetectorType::eDet, const size_t &cap = 65536);
        ~Triggers();
        TEWordQueue_t &GetQueue(){ return output_queue; }

        std::vector<Trigger_worker *>::iterator begin(){ return triggers.begin(); }
        std::vector<Trigger_worker *>::iterator end(){ return triggers.end(); }
    };

}

#endif //TDR2TREE_TRIGGERS_H
