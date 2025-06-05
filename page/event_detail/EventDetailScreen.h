#ifndef EVENTDETAILSCREEN_H
#define EVENTDETAILSCREEN_H

#include "EventDetailArgs.h"
#include "base/basescreen.h"

#include <QWidget>

namespace Ui {
class EventDetailScreen;
}

class Event;

class EventDetailScreen : public BaseScreen
{
    Q_OBJECT

public:
    explicit EventDetailScreen(const QString& tag = tag::EVENT_DETAIL_TAG, QWidget *parent = nullptr);
    ~EventDetailScreen();

    void onCreate(navigation::NavigationArgs* args) override;
    void reloadLocal() override;

    void bindEvent(std::shared_ptr<Event> event);

private:
    Ui::EventDetailScreen *ui;
    std::shared_ptr<Event> mCurEvent;
    std::shared_ptr<Event> mEventRef;       // Parent List's event, use it to update parent event if exists
    nucare::DatabaseManager* mRepository;
};

#endif // EVENTDETAILSCREEN_H
