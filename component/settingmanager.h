#ifndef SETTINGMANAGER_H
#define SETTINGMANAGER_H

#include "component.h"

class SubSettingItem; // Forward declaration
namespace nucare {
class DatabaseManager; // Forward declaration
}

namespace setting {

// Forward declaration for SettingManager to be used in ConfigEntry
class SettingManager;

SubSettingItem* buildSettingTree();

class ConfigEntry : public QObject { 
    Q_OBJECT
private:
    QVariant m_value;
    const char* m_key;
    SettingManager* m_manager = nullptr; // Pointer to the managing SettingManager

public:
    // Constructors (removed QObject* parent, manager will be the parent)
    explicit ConfigEntry(const char* key);
    ConfigEntry(const char* key, QVariant value);
    ConfigEntry(const char* key, SettingManager* manager);
    ConfigEntry(const char* key, QVariant value, SettingManager* manager);

    QVariant getValue() const;
    const char* getKey() const;

    // Setter for value, emits signal on change
    void setValue(QVariant newValue, bool saveNow = true);

    // Setter for manager (can be set after construction)
    void setManager(SettingManager* manager);

signals:
    // Signal emitted when the value of this config entry changes
    void itemValueChanged(SettingManager* manager, const char* key);

private:
    friend class SettingManager;
};

class SettingManager : public QObject, public Component
{
    Q_OBJECT
public:
    void initialize(); // Add initialize method
    void saveSetting(const QString& key, const QVariant& value); // Add saveSetting method
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const; // Add getSetting method

    static constexpr const char* KEY_INCREASE_TIME = "increaseTime";
 static constexpr const char* KEY_ACQTIME_BGR = "acqtime_background";
 static constexpr const char* KEY_ACQTIME_ID  = "acqtime_manualid";
 static constexpr const char* KEY_MEASURE_INVERVAL = "measure_interval";
 static constexpr const char* KEY_CALIB_COUNT = "calibration_totalcount";
 static constexpr const char* KEY_NDT_SRC = "ndt_source";
 static constexpr const char* KEY_PIPE_MATERIAL = "pipe_material";
 static constexpr const char* KEY_PIPE_DENSITY = "pipe_density";
 static constexpr const char* KEY_PIPE_THICKNESS = "pipe_thickness";
 static constexpr const char* KEY_PIPE_DIAMETER = "pipe_diameter";


 SettingManager(QObject* parent = nullptr);
 virtual ~SettingManager();

    // Method to retrieve a config entry by its key
    ConfigEntry* getEntry(const QString& key) const;

    // Specific getters for setting values
    int getIncreaseTime() const;
    int getAcqTimeBgr() const;
    int getAcqTimeId() const;
    int getMeasureInterval() const;
    int getCalibCount() const;
    QString getNdtSource() const;
    QString getPipeMaterial() const;
    double getPipeDensity() const;
    double getPipeThickness() const;
    double getPipeDiameter() const;

    template <class ... Args>
    auto subscribeKey(const QString& key, Args... args) {
        if (auto entry = getEntry(key)) {
            auto ret = connect(entry, &ConfigEntry::itemValueChanged, args...);
            emit entry->itemValueChanged(this, entry->m_key);
            return ret;
        }

        return QMetaObject::Connection();
    }

private slots:
    void onConfigEntryValueChanged(setting::SettingManager* manager, const char* key); // Slot to handle value changes

private:
    nucare::DatabaseManager* m_databaseManager = nullptr; // Pointer to DatabaseManager

    // Map to store config entries, allowing lookup by key
    QMap<QString, ConfigEntry*> m_settingItems;

    ConfigEntry* m_increaseTime;
    ConfigEntry* m_acqTimeBgr;
    ConfigEntry* m_acqTime;
    ConfigEntry* m_measureInterval;
    ConfigEntry* m_calibCount;
    ConfigEntry* m_ndtSource;
    ConfigEntry* m_pipeMaterial;
    ConfigEntry* m_pipeDensity;
    ConfigEntry* m_pipeThickness;
    ConfigEntry* m_pipeDiameter;

    void loadSettings();
};
}  // namespace setting

#endif
