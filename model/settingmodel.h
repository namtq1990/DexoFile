#ifndef SETTINGMODEL_H
#define SETTINGMODEL_H

#include <QAbstractListModel>
#include <QList>
#include "basesettingitem.h"
#include <memory>
#include "widget/basemodel.h"

class SettingModel : public BaseModel<BaseSettingItem*, SettingItemType>
{
    Q_OBJECT

public:
    SettingModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    void     setData(std::shared_ptr<QVector<BaseSettingItem*>> data) override;

    SettingItemType getType(const QModelIndex& index) const override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

   private slots:
    void onValueChanged(QVariant newValue);

};

#endif // SETTINGMODEL_H
