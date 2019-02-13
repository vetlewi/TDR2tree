#include "event.h"

#include "experimentsetup.h"

#include <TDirectory.h>
#include <TROOT.h>

#if !TEST_EVENT
ClassImp(Event_time_t)
ClassImp(Det_t)
ClassImp(UsrEvent_t)
#endif // !TEST_EVENT

Event_time_t::Event_time_t()
    : course_time( 0 )
    , fine_time( 0 ){ }

Event_time_t::Event_time_t(const int64_t &ct, const double &ft)
    : course_time( ct )
    , fine_time( ft ){ }

Event_time_t::Event_time_t(const Event_time_t &other)
    : course_time( other.course_time )
    , fine_time( other.fine_time ){ }

Event_time_t::~Event_time_t(){}


#if TEST_EVENT_2
#include <iostream>
int jain()
{

    // First now we will test Event_time_t!
    Event_time_t test1 = {12443, 54.3};

    std::cout << test1.course_time << " (expect 12443), ";
    std::cout << test1.fine_time << " (expect 54.3)" << std::endl;

    Event_time_t test2 = {12443, 54.0};
    test1 -= test2;
    std::cout << test1.course_time << " (expect 0), ";
    std::cout << test1.fine_time << " (expect 0.3)" << std::endl;

    Event_time_t test3 = test1 + test2;
    std::cout << test3.course_time << " (expect 12443), ";
    std::cout << test3.fine_time << " (expect 54.3)" << std::endl;

    test1 += test2;
    std::cout << test1.course_time << " (expect 12443), ";
    std::cout << test1.fine_time << " (expect 54.3)" << std::endl;

    Event_time_t test4 = test1 - test2;
    std::cout << test4.course_time << " (expect 0), ";
    std::cout << test4.fine_time << " (expect 0.3)" << std::endl;

    double test5 = test4;
    std::cout << test5 << " (expected 0.3)"  << std::endl;

    // Test Det_t

    Det_t test6 = {3, 54.3, 25.3, {104,44.3}};

    std::cout << test6.dnum << " (expect 3), ";
    std::cout << test6.energy << " (expect 54.3), ";
    std::cout << test6.timestamp << " (expect 148.3)." << std::endl;

    return 0;
}


#endif // TEST_EVENT
