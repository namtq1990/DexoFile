#include "EventListAdapter.h"
#include "util/util.h"
#include <QPainter>
#include <QDateTime>

using namespace std;

EventListAdapter::EventListAdapter(QObject *parent) : BaseModel<shared_ptr<Event>>(parent) {}

int EventListAdapter::columnCount(const QModelIndex &) const { return 3; }

QVariant EventListAdapter::data(const QModelIndex &index, int role) const {
    if (role != Qt::DisplayRole) return QVariant();

    auto event = rawData(index.row());
    if (event == nullptr) {
        if (mCb) {
            mCb->onRequestLoadMore(std::move(index));
            event = rawData(index.row());
        }
    }

    if (event == nullptr) return QString();

    switch (index.column()) {
        case 0:
            return QVariant::fromValue(event->getId());
        case 1: {
            auto timeStart = event->getStartedTime();
            return datetime::formatDate_yyyyMMdd_HHmm(timeStart);
        }
        case 2: {
            //        auto unit = mSettingDB->getDoseUnit();
            //        auto dose = nucare::from_nSvh(event->getDoses(), unit);
            return "";
            // return QString(nucare::autoformat_nSvh(event->getDoses()).c_str());
        }
        // case 3:
        // {
        //     QStringList isotopesStr;
        //     auto& isotopes = *event->isotopes();
        //     for (auto i = isotopes.begin(); i != isotopes.end(); i++) {
        //         isotopesStr.append(i->name.c_str());
        //     }

        //     return isotopesStr.join(',');
        // }
        default:
            return QString();
    }
}

QVariant EventListAdapter::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Orientation::Vertical) {
        return QVariant();
    } else if (orientation == Qt::Orientation::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return "No";
            case 1:
                return "Date";
            case 2:
                return "AvgDose";
                // case 3:
                //     return "Nuclide";
        }
    }

    return QAbstractTableModel::headerData(section, orientation, role);
}

////////////////////////////////////////////////////////////////////

void EventDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
    // if (index.column() == 0) {
    //     painter->setPen(event->isFavorite() ? QColor(0xFFA500) : QColor(Qt::darkCyan));
    // } else {
    //     painter->setPen(option.palette.text().color());
    // }

    QStyledItemDelegate::paint(painter, option, index);
}
