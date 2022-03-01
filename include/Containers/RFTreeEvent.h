//
// Created by Vetle Wegner Ingeberg on 01/03/2022.
//

#ifndef TDR2TREE_RFTREEEVENT_H
#define TDR2TREE_RFTREEEVENT_H

#include <TreeEvent.h>
#include <BasicStruct.h>

class RFTreeData {
    int entries;
    TBranch *entries_branch;

    VecBranch<uint16_t> ID;
    VecBranch<char> veto;
    VecBranch<char> cfd_fail;
    VecBranch<double> energy;
    VecBranch<double> time;

public:

    RFTreeData(TTree *tree, const char *name);

    void push_back(const word_t &event, const word_t &rfevent); // Add a new event

    template<class It>
    void add(It begin, It end, const word_t &rfevent){
        using std::placeholders::_1;
        std::for_each(begin, end, [this, rfevent](const word_t &p){ this->push_back(p, rfevent); });
    }

    inline void clear() noexcept {
        entries = 0;
        ID.clear();
        veto.clear();
        cfd_fail.clear();
        energy.clear();
        time.clear();
    }

    inline void validate() {
        ID.check_address();
        veto.check_address();
        cfd_fail.check_address();
        energy.check_address();
        time.check_address();
    }

};

class RFTreeEvent {
private:
    RFTreeData clover;
    RFTreeData labrL;
    RFTreeData labrS;
    RFTreeData labrF;
    RFTreeData ring;
    RFTreeData sect;
    RFTreeData back;

public:
    explicit RFTreeEvent(TTree *tree);

    void push_back(const word_t &event, const word_t &rfevent);

    RFTreeData &GetData(const DetectorType &type);
    RFTreeEvent &operator=(Event &event);

    inline void clear() noexcept
    {
        clover.clear();
        labrL.clear();
        labrS.clear();
        labrF.clear();
        ring.clear();
        sect.clear();
        back.clear();
    }

    inline void validate()
    {
        clover.validate();
        labrL.validate();
        labrS.validate();
        labrF.validate();
        ring.validate();
        sect.validate();
        back.validate();
    }

};

#endif //TDR2TREE_RFTREEEVENT_H
