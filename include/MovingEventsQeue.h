#ifndef MovingEventsQeue_H
#define MovingEventsQeue_H

#include <vector>

class MovingEventsQeue
{

private:
    /* data */
    std::vector<int> movingEvents;

public:
    MovingEventsQeue(/* args */)
    {
        movingEvents.reserve(10);
    }
    void AddEvent(int event)
    {
        if (movingEvents.size() > 0 &&
            movingEvents.back() == event)
        {
            return; // No change in move event
        }
        if (movingEvents.size() > 10)
        {
            movingEvents.erase(movingEvents.begin());
        }
        movingEvents.push_back(event);
    }
    std::vector<int> GetEvents()
    {
        return movingEvents;
    }
    void ClearEvents()
    {
        movingEvents.clear();
    }
    ~MovingEventsQeue()
    {
        movingEvents.clear();
    }
};

#endif