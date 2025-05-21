#include "settingmodel.h"
#include "basesettingitem.h"
#include <QDebug>
#include <QFont>
#include <QColor>
using namespace std;

SettingModel::SettingModel(QObject* parent) : BaseModel<BaseSettingItem*, SettingItemType>(parent) {
 }

QVariant SettingModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    BaseSettingItem* item = rawData(index.row());

    if (role == Qt::DisplayRole) {
        if (item->getShowValue()) {
            return QString("%1 (%2)").arg(item->getName()).arg(item->getValue().toString());
        } else {
            return item->getName();
        }
    } else if (role == Qt::FontRole) {
        if (item->getType() == SettingItemType::HeaderSettingItem) {
            QFont font;
            font.setPointSize(8);
            font.setBold(true);
            return font;
        }
    } else if (role == Qt::ForegroundRole) {
        if (item->getType() == SettingItemType::HeaderSettingItem) {
            return QColor(0x35b5e5);
        }
    }

    return QVariant();
}

SettingItemType SettingModel::getType(const QModelIndex& index) const { return rawData(index.row())->getType(); }

Qt::ItemFlags SettingModel::flags(const QModelIndex& index) const {
    auto item = rawData(index.row());
    if (item->getType() == SettingItemType::HeaderSettingItem) {
        return Qt::NoItemFlags;
    }

    return BaseModel::flags(index);
}

void SettingModel::onValueChanged(QVariant newValue)
{
    Q_UNUSED(newValue);
    QModelIndex index = QModelIndex();
    for (int row = 0; row < rowCount(); ++row) {
        BaseSettingItem* item = rawData(row);
        if (sender() == item) {
            index = this->index(row, 0);
            break;
        }
    }

    if (index.isValid()) {
        emit dataChanged(index, index);
    }
}

void SettingModel::setData(std::shared_ptr<QVector<BaseSettingItem*>> data) {
    if (mData == data)
        return;

    beginResetModel();
    if (mData) {
        for (BaseSettingItem* item : *mData) {
            if (item) disconnect(item, &BaseSettingItem::valueChanged, this, &SettingModel::onValueChanged);
        }
    }
    mData = data;
    if (mData) {
        for (BaseSettingItem* item : *mData) {
            if (item) connect(item, &BaseSettingItem::valueChanged, this, &SettingModel::onValueChanged);
        }
    }
    endResetModel();
}
