#ifndef SETTINGITEMDELEGATE_H
#define SETTINGITEMDELEGATE_H

#include <QStyledItemDelegate>

class SettingItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    SettingItemDelegate(QObject *parent = nullptr);

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

#endif // SETTINGITEMDELEGATE_H