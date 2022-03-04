//
// Created by Vetle Wegner Ingeberg on 01/03/2022.
//

#ifndef TDR2TREE_RFTREEEVENT_H
#define TDR2TREE_RFTREEEVENT_H

#include <TreeEvent.h>
#include <BasicStruct.h>

class RFTreeData {
    ObjBranch<int> entries;
    //int entries;
    //TBranch *entries_branch;

    VecBranch<uint16_t> ID;
    VecBranch<char> veto;
    VecBranch<char> cfd_fail;
    VecBranch<double> energy;
    VecBranch<double> time;
    VecBranch<int64_t> timestamp;
    VecBranch<double> cfdcorr;
    VecBranch<uint16_t> raw_cfd;
    VecBranch<uint16_t> cfd_cross;
    VecBranch<uint16_t> cfd_val;

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
        timestamp.clear();
        cfdcorr.clear();
        cfd_fail.clear();
        cfd_cross.clear();
        cfd_val.clear();
    }

    inline void validate() {
        ID.check_address();
        veto.check_address();
        cfd_fail.check_address();
        energy.check_address();
        time.check_address();
        timestamp.check_address();
        cfdcorr.check_address();
        raw_cfd.check_address();
        cfd_cross.check_address();
        cfd_val.check_address();
    }

};

class TriggerData {
private:
    ObjBranch<uint16_t> ID;
    ObjBranch<char> veto;
    ObjBranch<char> cfd_fail;
    ObjBranch<double> energy;
    ObjBranch<int64_t> timestamp;
    ObjBranch<double> cfdcorr;
    ObjBranch<uint16_t> raw_cfd;
    ObjBranch<uint16_t> cfd_cross;
    ObjBranch<uint16_t> cfd_val;

public:
    TriggerData(TTree *tree, const char *name);
    void Set(const word_t &word);

    inline void clear()
    {
        ID = 0;
        veto = 0;
        cfd_fail = 0;
        energy = 0;
        timestamp = 0;
        cfdcorr = 0;
        raw_cfd = 0;
        cfd_cross = 0;
        cfd_val = 0;
    }

    inline void validate(){ /* nop */ }

    TriggerData &operator=(const word_t &data)
    {
        Set(data);
        return *this;
    }

};

class RFTreeEvent {
private:
    TriggerData trigger;
    RFTreeData clover;
    RFTreeData labrL;
    RFTreeData labrS;
    RFTreeData labrF;
    RFTreeData ring;
    RFTreeData sect;
    RFTreeData back;
    RFTreeData rf;

public:
    explicit RFTreeEvent(TTree *tree);

    void push_back(const word_t &event, const word_t &rfevent);

    RFTreeData &GetData(const DetectorType &type);
    RFTreeEvent &operator=(Event &event);

    inline void clear() noexcept
    {
        trigger.clear();
        clover.clear();
        labrL.clear();
        labrS.clear();
        labrF.clear();
        ring.clear();
        sect.clear();
        back.clear();
        rf.clear();
    }

    inline void validate()
    {
        trigger.validate();
        clover.validate();
        labrL.validate();
        labrS.validate();
        labrF.validate();
        ring.validate();
        sect.validate();
        back.validate();
        rf.validate();
    }

};

#endif //TDR2TREE_RFTREEEVENT_H
