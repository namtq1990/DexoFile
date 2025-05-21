#ifndef BASESETTINGITEM_H
#define BASESETTINGITEM_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QFlags>
#include <QAbstractItemModel>
#include <memory>

enum class SettingItemType
{
    SubSettingItem,
    HeaderSettingItem,
    ChoiceSettingItem,
    EditorSettingItem,
    InfoItem
};

class BaseSettingItem : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariant value READ getValue WRITE setValue NOTIFY valueChanged)

   public:
    virtual SettingItemType getType() const { return SettingItemType::InfoItem; };

   protected:
    QString     mKey         = nullptr;
    QString     mName        = nullptr;
    QString     mIcon        = nullptr;
    const char* mClickAction = nullptr;
    bool        mShowValue   = false;
    bool        mEnabled     = true;
    QVariant    mValue;

   public:
    BaseSettingItem(QObject* parent = nullptr);
    BaseSettingItem(const BaseSettingItem& item);

    QString  getKey() const { return mKey; }
    QString  getName() const { return mName; }
    QString  getIcon() const { return mIcon; }
    const char*  getAction() const { return mClickAction; }
    bool     getShowValue() const { return mShowValue; }
    bool     isEnabled() const { return mEnabled; }
    QVariant getValue() const { return mValue; }

    void setEnabled(bool isEnabled);

    BaseSettingItem* setKey(const QString& key);
    BaseSettingItem* setName(const QString& name);
    BaseSettingItem* setIcon(const QString& icon);
    BaseSettingItem* setClickAction(const char* actionName);
    BaseSettingItem* setShowValue(bool show);
    void             setValue(QVariant value);

   Q_SIGNALS:
    void clicked(BaseSettingItem* p);
    void valueChanged(QVariant newValue);

   public slots:
    void onClicked();
};

class SubSettingItem : public BaseSettingItem
{
    Q_OBJECT
   public:
    SubSettingItem(QObject* parent = nullptr);
    SettingItemType getType() const override { return SettingItemType::SubSettingItem; }

    SubSettingItem*                            setSettings(QVector<BaseSettingItem*>&& nodes);
    std::shared_ptr<QVector<BaseSettingItem*>> getNodes();

   private:
    std::shared_ptr<QVector<BaseSettingItem*>> mNodes;
};

class HeaderSettingItem : public BaseSettingItem
{
    Q_OBJECT
   public:
    HeaderSettingItem(QObject* parent = nullptr) : BaseSettingItem(parent) {}
    SettingItemType getType() const override { return SettingItemType::HeaderSettingItem; }
};

class ChoiceSettingItem : public BaseSettingItem
{
    Q_OBJECT
   public:
    ChoiceSettingItem(QMap<QString, QVariant>&& choices, QObject* parent = nullptr);
    SettingItemType getType() const override { return SettingItemType::ChoiceSettingItem; }

   private:
    QMap<QString, QVariant> mData;
};

class EditorSettingItem : public BaseSettingItem
{
    Q_OBJECT
   public:
    EditorSettingItem(QObject* parent = nullptr);
    SettingItemType getType() const override { return SettingItemType::EditorSettingItem; }
};

class InfoSettingItem : public BaseSettingItem
{
    Q_OBJECT
   public:
    InfoSettingItem(QObject* parent = nullptr) : BaseSettingItem(parent) {}
    SettingItemType getType() const override { return SettingItemType::InfoItem; }
};

#endif  // BASESETTINGITEM_H
