//
// Created by Vetle Wegner Ingeberg on 11/03/2022.
//

#ifndef THREADSAFEEVENT_H
#define THREADSAFEEVENT_H

#include <Event.h>
#include <ThreadSafeHistograms.h>

class ThreadSafeEvent
{
private:
    word_t trigger;
    std::vector<word_t> event_data;
    std::map<DetectorType, Subevent> type_bounds;

public:

    //! Constructor.
    explicit ThreadSafeEvent(std::vector<word_t> event, ThreadSafeHistogram2D *ab_t_clover = nullptr);
    explicit ThreadSafeEvent(word_t trigger, std::vector<word_t> event, ThreadSafeHistogram2D *ab_t_clover = nullptr);

    template<class It>
    ThreadSafeEvent(It begin, It end, ThreadSafeHistogram2D *ab_t_clover = nullptr) : event_data(begin, end){ index(ab_t_clover); }

    // Make public such that we can rebuild the event whenever needed.
    void index(ThreadSafeHistogram2D *ab_t_clover);

    inline Subevent &GetDetector(const DetectorType &type){ return type_bounds.at(type); }

    inline bool IsTriggered() const { return trigger.address != 0; }
    inline word_t GetTrigger() const { return trigger; }

};

#endif //TDR2TREE_THREADSAFEEVENT_H
