#include "settingitemdelegate.h"
#include <QPainter>
#include "model/settingmodel.h"
#include <QStyleOptionViewItem>
#include <QPixmap>

SettingItemDelegate::SettingItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

void SettingItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto settingModel = qobject_cast<const SettingModel*>(index.model());
    SettingItemType type = SettingItemType::InfoItem;
    QString iconPath;
    if (settingModel) {
        type = settingModel->getType(index);
        BaseSettingItem* item = settingModel->rawData(index.row());
        if (item && type != SettingItemType::HeaderSettingItem) {
            iconPath = item->getIcon();
        }
    }

    QStyleOptionViewItem newOption = option;
    int iconSize = 12;
    int iconMargin = 8;
    if (type != SettingItemType::HeaderSettingItem) {
        QRect iconRect(option.rect.left() + iconMargin, option.rect.center().y() - iconSize/2, iconSize, iconSize);
        if (!iconPath.isEmpty()) {
            QPixmap iconPixmap(iconPath);
            painter->drawPixmap(iconRect, iconPixmap.scaled(iconSize, iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        // Always reserve space for icon, even if missing
        newOption.rect.setLeft(iconRect.right() + iconMargin);
    } else {
        newOption.rect.adjust(5, 0, 0, 0);
    }
    QStyledItemDelegate::paint(painter, newOption, index);
}
