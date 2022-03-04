//
// Created by Vetle Wegner Ingeberg on 15/04/2021.
//

#ifndef TDR2TREE_TREEEVENT_H
#define TDR2TREE_TREEEVENT_H

#include <vector>
#include <string>
#include <iostream>

#include <TTree.h>
#include <TBranch.h>

#include <experimentsetup.h>

struct word_t;
class Event;

class TTree;
class TBranch;

enum type_map : char {
    char_array_type = 'C',
    signed_char_type = 'B',
    unsigned_char_type = 'b',
    signed_short_type = 'S',
    unsigned_short_type = 's',
    signed_integer_type = 'I',
    unsigned_integer_type = 'i',
    signed_long_type = 'L',
    unsigned_long_type = 'l',
    float_type = 'F',
    double_type = 'D',
    bool_type = 'O'
};

std::string CName(const char *base_name, const char *identifier);
std::string CLeaf(const char *base_name, const char *identifier, const char type);


template<typename T>
class VecBranch {
    T container[256];
    //std::vector<T> container;
    TBranch *branch;

public:

    VecBranch(TTree *tree, const char *name, const char *leaflist)
        : container{ 0 } // Initialize here to ensure that memory is allocated.
        , branch( tree->Branch(name, container/*container.data()*/, leaflist) ){ /*container.clear();*/ }

    inline void push_back(const T &val, const size_t &num){
        container[num] = val;
    }

    inline void clear() noexcept {
        //container.clear();
    }

    // To be called before 'Fill' method of the tree is called.
    // Essentially we need to ensure that the address of the underlying container (vector) isn't changed,
    // and if so we will need to change it to the correct address.
    inline void check_address() {
        size_t addr = size_t(reinterpret_cast<const char *>(container));
        size_t baddr = size_t(branch->GetAddress());
        if ( reinterpret_cast<const char *>(container) != branch->GetAddress() )
            std::cout << "Got addresses: " << size_t(reinterpret_cast<const char *>(container)) << " " << size_t(branch->GetAddress()) << std::endl;
    }

};

class TreeData {
    int entries;
    TBranch *entries_branch;

    VecBranch<uint16_t> ID;
    VecBranch<char> veto;
    VecBranch<char> cfd_fail;
    VecBranch<double> energy;
    VecBranch<double> cfd_corr;
    VecBranch<int64_t> timestamp;

public:

    TreeData(TTree *tree, const char *name);

    void push_back(const word_t &event); // Add a new event

    template<class It>
    void add(It begin, It end){
        using std::placeholders::_1;
        std::for_each(begin, end, [this](const word_t &p){ this->push_back(p); });
    }

    inline void clear() noexcept {
        entries = 0;
        ID.clear();
        veto.clear();
        cfd_fail.clear();
        energy.clear();
        cfd_corr.clear();
        timestamp.clear();
    }

    inline void validate() {
        ID.check_address();
        veto.check_address();
        cfd_fail.check_address();
        energy.check_address();
        cfd_corr.check_address();
        timestamp.check_address();
    }

};

class TreeEvent {
    TreeData clover;
    TreeData labrL;
    TreeData labrS;
    TreeData labrF;
    TreeData ring;
    TreeData sect;
    TreeData back;
    TreeData rf;

public:

    explicit TreeEvent(TTree *tree);

    void push_back(const word_t &event);

    TreeData &GetData(const DetectorType &type);
    TreeEvent &operator=(Event &event);

    inline void clear() noexcept
    {
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


#endif //TDR2TREE_TREEEVENT_H
