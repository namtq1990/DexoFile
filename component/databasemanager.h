#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include "component/component.h"
#include <QSqlDatabase>
#include <QObject>
#include <QString>
#include <memory>
#include <vector>

// Forward declarations for model classes
class Background;
class Calibration;
class Event;
class DetectorInfo;
class DetectorCalibConfig;

namespace nucare {

class DatabaseManager : public QObject, public Component
{
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    void initialize(const QString& dataDirPath);
    QSqlDatabase database() const;

    // Detector operations
    std::shared_ptr<DetectorInfo> getDetectorById(int id); // Return shared_ptr, throw on error/not found
    std::shared_ptr<DetectorInfo> getDetectorByCriteria(
        const QString &serialNumber,
        const QString &model,
        const QString &probeType,
        const QString &detectorCode,
        const QString &crystalType
    ); // Return shared_ptr, throw on error/not found
    std::shared_ptr<DetectorInfo> getLastDetector(); // Return shared_ptr, throw on error/not found

    // Background operations
    std::shared_ptr<Background> getBackgroundById(int id);
    std::shared_ptr<Background> getLatestBackground(int detectorId); // Added this line

    // Calibration operations
    std::shared_ptr<Calibration> getCalibrationById(int id);
    std::shared_ptr<Calibration> getLatestCalibration(int detectorId);

    // Event operations
    std::vector<std::shared_ptr<Event>> getEvents(int page, int pageSize);
    std::shared_ptr<Event> getEventDetails(int id);
    void loadEventDetailsInto(Event* event);
    std::shared_ptr<DetectorCalibConfig> getDefaultDetectorConfig(const int detId);

    // Insert operations (using raw pointers as requested)
    int insertDetector(const DetectorInfo* detector);
    int insertBackground(const Background* background);
    int insertCalibration(const Calibration* calibration);
    qlonglong insertEvent(const Event* event);
    int insertDetectorCalibConfig(const DetectorCalibConfig* config);
    qlonglong insertEventDetail(qlonglong eventId, const QString& spectrumData);

    // Settings operations
    QVariant getSetting(const QString& name, const QVariant& defaultValue = QVariant());
    void setSetting(const QString& name, const QVariant& value);
    QMap<QString, QVariant> getAllSettings();

private:
    QSqlDatabase m_database;
    QString m_dataDirPath;

    // Helper for executing queries and fetching a single row
    QSqlQuery executeSingleRowQuery(const QString& queryString, const QVariantMap& bindValues);
    bool executeQuery(QSqlQuery& query, const QString& context);

    bool deployDatabase(const QString& sourcePath, const QString& destinationPath);
    bool createTablesIfNotExist();
};

} // namespace nucare

#endif // DATABASEMANAGER_H
