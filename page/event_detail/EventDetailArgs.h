#ifndef EVENTDETAILARGS_H
#define EVENTDETAILARGS_H

#include "component/navigationcomponent.h"

namespace nucare {
    class DatabaseManager;
}

class Event;

struct EventDetailArgs : public navigation::NavigationArgs {
    int eventID = 0;
    nucare::DatabaseManager* repository;
    std::shared_ptr<Event> event = NULL;

    EventDetailArgs(const int eventID,
                    nucare::DatabaseManager* repository,
                    std::shared_ptr<Event> event = NULL)
        : eventID(eventID),
          repository(repository),
          event(event)
    {}
};

#endif // EVENTDETAILARGS_H
