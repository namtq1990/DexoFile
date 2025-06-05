#ifndef EVENTLISTSCREEN_H
#define EVENTLISTSCREEN_H

#include "base/basescreen.h"
#include "page/events/EventListAdapter.h"
#include <QWidget>

namespace Ui {
class EventListScreen;
}
namespace nucare { class DatabaseManager; }

class Event;

class EventListScreen : public BaseScreen, public EventListCallback
{
    Q_OBJECT
private:
    EventListAdapter* mAdapter;
public:
    explicit EventListScreen(QWidget *parent = nullptr, const QString& tag = tag::EVENTS_TAG);
    ~EventListScreen();

    void onCreate(navigation::NavigationArgs* args) override;
    void onRequestLoadMore(const QModelIndex &&index) override;
    void reloadLocal() override;


    void toEventDetail(long id);
    void onLoadEvent(QVector<std::shared_ptr<Event>>&& events, int page);
private:
    Ui::EventListScreen *ui;
    
    int mTotalCount = 0;
    nucare::DatabaseManager* mDataRepo;
    std::shared_ptr<QVector<std::shared_ptr<Event>>> mEvents;

    struct LoadingSession {
        int start = -1;
        int end = -1;
    } mLoadSession;
};

#endif // EVENTLISTSCREEN_H
