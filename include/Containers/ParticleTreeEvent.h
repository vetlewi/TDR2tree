//
// Created by Vetle Wegner Ingeberg on 14/03/2022.
//

#ifndef TDR2TREE_PARTICLETREEEVENT_H
#define TDR2TREE_PARTICLETREEEVENT_H

#include <TreeEvent.h>
#include <RFTreeEvent.h>

struct ParticleEvent;

class ParticleData {
private:
    TriggerData ring;
    ObjBranch<double> ringTime;
    TriggerData sect;
    ObjBranch<double> sectTime;
    TriggerData back;
    ObjBranch<double> backTime;

public:
    ParticleData(TTree *tree);
    void Set(const ParticleEvent &event_data);

    inline void clear()
    {
        ring.clear();
        sect.clear();
        back.clear();
    }

    ParticleData &operator=(const ParticleEvent &event_data)
    {
        Set(event_data);
        return *this;
    }

};

class ParticleTreeData {
private:
    ObjBranch<int> entries;
    VecBranch<uint16_t> ID;
    VecBranch<char> veto;
    VecBranch<char> cfd_fail;
    VecBranch<double> energy;
    VecBranch<double> time;
    VecBranch<int64_t> timestamp;
    VecBranch<double> cfdcorr;

public:
    ParticleTreeData(TTree *tree, const char *name);

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
    }

    inline void validate() {
        ID.check_address();
        veto.check_address();
        cfd_fail.check_address();
        energy.check_address();
        time.check_address();
        timestamp.check_address();
        cfdcorr.check_address();
    }

};

class ParticleTreeEvent {
private:
    TriggerData trigger;
    ParticleTreeData clover;
    ParticleTreeData labrL;
    ParticleTreeData labrS;
    ParticleTreeData labrF;
    ParticleTreeData rf;
    ParticleData particle;

    bool have_valid_particle;

public:
    explicit ParticleTreeEvent(TTree *tree);

    void push_back(const word_t &event, const word_t &trigger);

    ParticleTreeEvent &operator=(const Event &event);
    ParticleTreeData &GetData(const DetectorType &type);

    inline void clear() noexcept
    {
        trigger.clear();
        clover.clear();
        labrL.clear();
        labrS.clear();
        labrF.clear();
        rf.clear();
        particle.clear();
    }

    bool validate()
    {
        trigger.validate();
        clover.validate();
        labrL.validate();
        labrS.validate();
        labrF.validate();
        rf.validate();
        return have_valid_particle;
    }

};

#endif //TDR2TREE_PARTICLETREEEVENT_H
