#include "EventListScreen.h"
#include "ui_EventListScreen.h"
#include "component/componentmanager.h"
#include "component/databasemanager.h"
#include "model/Event.h"
#include "page/event_detail/EventDetailArgs.h"
#include "EventListAdapter.h"

using namespace std;
#define EVENT_PAGE_SIZE 15

navigation::NavigationEntry* navigation::toEventList(BaseView* parent, const QString& tag) {
    auto widget = dynamic_cast<QWidget*>(parent);
    auto view = new EventListScreen(widget);
    auto ret = new NavigationEntry(CHILD_IN_WINDOW, view, nullptr, widget->parent());

    parent->getNavigation()->enter(ret);

    return ret;
}

EventListScreen::EventListScreen(QWidget *parent, const QString &tag)
    : BaseScreen(tag, parent),
      ui(new Ui::EventListScreen)
{
    mDataRepo = ComponentManager::instance().databaseManager().data();

    ui->setupUi(this);
    mAdapter = new EventListAdapter(ui->eventList);
    mAdapter->setCallback(this);
    ui->eventList->setModel(mAdapter);
    ui->eventList->setItemDelegate(new EventDelegate(this));
    ui->eventList->setAutoSelect(true);
    ui->eventList->setClickListener([&](const QModelIndex& index) {
        auto id = index.data().value<long>();
        toEventDetail(id);
    });
    if (auto header = ui->eventList->horizontalHeader()) {
        header->setSectionResizeMode(0, QHeaderView::ResizeMode::ResizeToContents);
        header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        header->setSectionResizeMode(2, QHeaderView::Stretch);
//        header->setSectionResizeMode(3, QHeaderView::ResizeMode::Interactive);

        header->resizeSection(1, 80);

        QString style = R"( QHeaderView::section {
                                border: none;
                                border-bottom: 1px solid #888888;
                                background-color: black;
                                color: darkcyan;
                                padding-bottom: 2px;
                            }
                          )";

        header->setStyleSheet(style);

        header->show();
    }
    if (auto vHeader = ui->eventList->verticalHeader()) {
        vHeader->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    }

    setLeftAction(new ViewAction ("", [this]() {
                          getNavigation()->pop();
                          return true;
                      }, ":/icons/menu_home"));
    setCenterAction(new ViewAction ("", [this]() {
                            ui->eventList->moveUp();
                            return true;
                        }, ":/icons/menu_up_ok"));
    setLongCenterAction(new ViewAction ("", [this]() {
                                ui->eventList->performClick();
                                return true;
                            }));
    setRightAction(new ViewAction ("", [this]() {
                           ui->eventList->moveDown();
                           return true;
                       }, ":/icons/menu_down"));
}

EventListScreen::~EventListScreen()
{
    delete ui;
}

void EventListScreen::onCreate(navigation::NavigationArgs *args)
{
    BaseScreen::onCreate(args);
    mTotalCount = mDataRepo->getTotalEventCount();
    mEvents = make_shared<QVector<shared_ptr<Event>>>(mTotalCount, nullptr);
    auto events = mDataRepo->getEvents(0, EVENT_PAGE_SIZE);
    onLoadEvent(std::move(events), 0);
    mAdapter->setData(mEvents);
}

void EventListScreen::onRequestLoadMore(const QModelIndex &&qIndex)
{
    auto index = qIndex.row();
    if (index < 0) return;

        int page = index / EVENT_PAGE_SIZE;
        int pageStart = page * EVENT_PAGE_SIZE;
        int pageEnd = min(pageStart + EVENT_PAGE_SIZE, (int) mEvents->size());
        if (pageEnd > pageStart) {
            pageEnd--;      // Convert page end into index
        }

        if (mLoadSession.start == pageStart
                && mLoadSession.end == pageEnd) return;

        mLoadSession.start = pageStart;
        mLoadSession.end = pageEnd;

        auto events = mDataRepo->getEvents(page, EVENT_PAGE_SIZE);
        onLoadEvent(std::move(events), page);
}

void EventListScreen::reloadLocal()
{
    ui->retranslateUi(this);
}

void EventListScreen::toEventDetail(long id) {
    auto event = std::find_if(mEvents->begin(), mEvents->end(), [id](std::shared_ptr<Event> p) {
        return p && p->getId() == id;
    });
    auto ret   = new EventDetailArgs(id, mDataRepo);
    if (event != mEvents->end()) {
        ret->event = *event;
    }

    navigation::toEventDetail(this, ret);
}

void EventListScreen::onLoadEvent(QVector<std::shared_ptr<Event>> &&events, int page)
{
    if (mEvents->size() <= events.size()) {
        mEvents = make_shared<QVector<shared_ptr<Event>>>(events);
        return;
    }

    int pageIndex = page * EVENT_PAGE_SIZE;
    auto destBegin = mEvents->begin() + pageIndex;
    auto destEnd = destBegin + std::min(events.size(), mEvents->size() - pageIndex);
    std::copy(events.begin(), events.begin() + (destEnd - destBegin), destBegin);

    // Don't need to update the view in this one, because this one is called from Model to get data
    // And Model can bind UI and data at same time
    // So update model here is quite wasting resource
}


