#include "basesettingitem.h"
#include <QVariant>
#include <QCoreApplication>

BaseSettingItem::BaseSettingItem(QObject *parent) : QObject(parent) {
    // Default constructor
}

BaseSettingItem::BaseSettingItem(const BaseSettingItem &item)
    : QObject(item.parent()),
      mKey(item.mKey),
      mName(item.mName),
      mIcon(item.mIcon),
      mShowValue(item.mShowValue),
      mEnabled(item.mEnabled),
      mValue(item.mValue)
{
    // Copy constructor
}

void BaseSettingItem::setEnabled(bool isEnabled) { mEnabled = isEnabled; }

BaseSettingItem* BaseSettingItem::setKey(const QString& key) { mKey = key; return this; }
BaseSettingItem* BaseSettingItem::setName(const QString& name) { mName = name; return this; }
BaseSettingItem* BaseSettingItem::setIcon(const QString& icon) { mIcon = icon; return this; }
BaseSettingItem* BaseSettingItem::setShowValue(bool show) { mShowValue = show; return this; }

void BaseSettingItem::setValue(QVariant value) {
    if (mValue != value) {
        mValue = value;
        emit valueChanged(value);
    }
}

void BaseSettingItem::onClicked()
{
    emit clicked();
}

SubSettingItem* SubSettingItem::setSettings(QVector<BaseSettingItem*>&& nodes) {
    mNodes = std::make_shared<QVector<BaseSettingItem*>>(nodes);
}
