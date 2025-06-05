#ifndef EVENTLISTADAPTER_H
#define EVENTLISTADAPTER_H

#include "widget/basemodel.h"
#include "model/Event.h"
#include <QStyledItemDelegate>

struct EventListCallback {
    virtual void onRequestLoadMore(const QModelIndex&& index) = 0;
};

class EventListAdapter : public BaseModel<std::shared_ptr<Event>>
{
    Q_OBJECT
private:
    EventListCallback* mCb = nullptr;
public:
    EventListAdapter(QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent) const override;
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void setCallback(EventListCallback* cb) { mCb = cb; }
};


class EventDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    EventDelegate(QWidget* parent = NULL) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // EVENTLISTADAPTER_H
