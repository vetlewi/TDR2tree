//
// Created by Vetle Wegner Ingeberg on 23/03/2020.
//

#ifndef XIATYPES_H
#define XIATYPES_H

struct XIA_base_header {
    unsigned chanID : 4;
    unsigned slotID : 4;
    unsigned crateID : 4;
    unsigned headerLen : 5;
    unsigned eventLen : 14;
    bool finish_code : 1;
    unsigned evttime_lo : 32;
    unsigned evttime_hi : 16;
    unsigned CFD_result : 16;
    unsigned event_energy : 16;
};

struct


#endif // XIATYPES_H
