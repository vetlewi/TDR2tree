#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <vector>


#define TEST_EVENT 1

#if !TEST_EVENT
#include <TObject.h>
#endif // !TEST_EVENTS

// This is the event class. We need this!

#if TEST_EVENT
class Event_time_t
#else
class Event_time_t : public TObject
#endif // TEST_EVENT
{
public:
    int64_t course_time;
    double fine_time;

    Event_time_t();

    Event_time_t(const int64_t &ct, const double &ft);
        //: course_time( ct )
        //, fine_time( ft ){ }

    Event_time_t(const Event_time_t &other);
        //: course_time( other.course_time )
        //, fine_time( other.fine_time ){ }

    virtual ~Event_time_t();

    inline Event_time_t &operator-=(const Event_time_t &other)
    {
        course_time -= other.course_time;
        fine_time -= other.fine_time;
        return *this;
    }

    inline Event_time_t &operator+=(const Event_time_t &other)
    {
        course_time += other.course_time;
        fine_time += other.fine_time;
        return *this;
    }

    inline Event_time_t &operator=(const Event_time_t &other)
    {
        if ( this != &other ){
            course_time = other.course_time;
            fine_time = other.fine_time;
        }
        return *this;
    }

    inline Event_time_t &operator=(Event_time_t &&other) noexcept
    {
        if ( this != &other ){
            course_time = other.course_time;
            fine_time = other.fine_time;
        }
        return *this;
    }

    friend Event_time_t operator+(Event_time_t lhs, const Event_time_t &rhs)
    {
        lhs += rhs;
        return lhs;
    }

    friend Event_time_t operator-(Event_time_t lhs, const Event_time_t &rhs)
    {
        lhs -= rhs;
        return lhs;
    }

    inline operator double() const { return course_time + fine_time; }
#if !TEST_EVENT
    ClassDef(Event_time_t,3)
#endif // !TEST_EVENT
};


#if TEST_EVENT
class Det_t
#else
class Det_t : public TObject
#endif // TEST_EVENT
{
public:
    uint16_t dnum; // Detector number. (eg. ring number)
    double energy; // Detected energy
    double angle; // Angle with respect to the beam line
    //Event_time_t timestamp; // Time of the detector event.

    int64_t time_L;
    double time_S;

    Det_t(){ }

    Det_t(const uint16_t &d, const double &e, const double &a, const int64_t &tL, const double &tS)
        : dnum( d )
        , energy( e )
        , angle( a )
        , time_L( tL )
        , time_S( tS ){ }

    Det_t(const Det_t &other)
        : dnum( other.dnum )
        , energy( other.energy )
        , angle( other.angle )
        , time_L( other.time_L )
        , time_S( other.time_S ){ }

    inline Det_t &operator=(const Det_t &other)
    {
        dnum = other.dnum;
        energy = other.energy;
        angle = other.angle;
        time_L = other.time_L;
        time_S = other.time_S;
        return *this;
    }

#if !TEST_EVENT
    ClassDef(Det_t,3)
#endif // !TEST_EVENT
};

#if TEST_EVENT
class UsrEvent_t
#else
class UsrEvent_t : public TObject
#endif // !TEST_EVENT
{
public:

    // 'Trigger' event
    Det_t ring;

    // Vector containing all the dE sector events.
    std::vector<Det_t> sect;

    // Vector containing all the E events.
    std::vector<Det_t> back;

    // Vector containing all the large volume LaBr3:Ce events.
    std::vector<Det_t> labr_L;

    // Vector containing all the small volume LaBr3:Ce events.
    std::vector<Det_t> labr_S;

    // Vector containing all the CLOVER events.
    std::vector<Det_t> clover;

    // Reset everything
    inline void Reset()
    {
        sect.clear();
        back.clear();
        labr_L.clear();
        labr_S.clear();
        clover.clear();
    }
#if !TEST_EVENT
    ClassDef(UsrEvent_t,3)
#endif // !TEST_EVENT
};


#endif // EVENT_H
