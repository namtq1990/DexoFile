#include "databasemanager.h"
#include "component/componentmanager.h"
#include "util/util.h" // For logging
#include "model/Background.h"
#include "model/Calibration.h"
#include "model/Event.h"
#include "model/DetectorInfo.h"
#include "model/DetectorCalibConfig.h"
#include "model/DetectorCode.h" // For DetectorCode and CrystalType
#include "model/Spectrum.h"     // For Spectrum_t
#include "model/Time.h"         // For nucare::Timestamp
#include "model/Types.h"        // For Coeffcients

#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QSqlError>
#include <QCoreApplication>
#include <QSqlQuery>
#include <QVariant>
#include <QVariantMap>

namespace nucare {

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), Component("DATABASE")
{
}

bool DatabaseManager::executeQuery(QSqlQuery& query, const QString& context) {
    QString sql = query.lastQuery();
    for (const auto& key : query.boundValues().keys()) {
        sql.replace(key, query.boundValue(key).toString());
    }

    logD() << "Executing SQL [" << context << "]: " << sql;

    bool success = query.exec();
    if (!success) {
        logE() << "SQL execution failed [" << context << "]: " << query.lastError().text();
        logE() << "Failed SQL: " << sql;
    } else {
        logD() << "SQL executed successfully [" << context << "]";
    }

    return success;
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen()) {
        // Use the connection name to remove the database
        QString connectionName = m_database.connectionName();
        m_database.close();
        QSqlDatabase::removeDatabase(connectionName);
        logI() << "Database connection closed and removed.";
    }
}

void DatabaseManager::initialize(const QString& dataDirPath)
{
    m_dataDirPath = dataDirPath;
    QString dbFileName = "NDT.db";
    QString deployedDbPath = m_dataDirPath + QDir::separator() + dbFileName;

    QString sourceDbPath = QStandardPaths::locate(QStandardPaths::DataLocation, dbFileName, QStandardPaths::LocateFile);
    if (sourceDbPath.isEmpty()) {
        sourceDbPath = QCoreApplication::applicationDirPath() + "/../share/NDT/" + dbFileName;
        if (!QFile::exists(sourceDbPath)) {
            sourceDbPath = ":/NDT.db";
            logW() << "Using Qt resource for NDT.db as fallback. Please ensure CMake deploys NDT.db to a system location.";
        }
    }

    if (!QFile::exists(deployedDbPath)) {
        logI() << "Database not found at" << deployedDbPath << ". Deploying from" << sourceDbPath;
        if (!deployDatabase(sourceDbPath, deployedDbPath)) {
            logE() << "Failed to deploy database to" << deployedDbPath;
            return;
        }
    } else {
        logI() << "Database already exists at" << deployedDbPath;
    }

    // Use a unique connection name to avoid issues with default connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", "NDT_DB_Connection");
    m_database.setDatabaseName(deployedDbPath);

    if (!m_database.open()) {
        logE() << "Failed to open database:" << m_database.lastError().text();
    } else {
        logI() << "Database opened successfully at" << deployedDbPath;
        createTablesIfNotExist(); // Ensure tables exist after opening
    }
}

QSqlDatabase DatabaseManager::database() const
{
    return m_database;
}

bool DatabaseManager::deployDatabase(const QString& sourcePath, const QString& destinationPath) {
    QDir destDir = QFileInfo(destinationPath).dir();
    if (!destDir.exists()) {
        if (!destDir.mkpath(".")) {
            logE() << "Failed to create directory for database:" << destDir.absolutePath();
            return false;
        }
    }

    if (QFile::exists(destinationPath)) {
        if (!QFile::remove(destinationPath)) {
            logW() << "Failed to remove existing database file at" << destinationPath;
        }
    }

    if (!QFile::copy(sourcePath, destinationPath)) {
        logE() << "Failed to copy database from" << sourcePath << "to" << destinationPath << ":" << QFile::copy(sourcePath, destinationPath);
        return false;
    }
    return true;
}

bool DatabaseManager::createTablesIfNotExist()
{
    QSqlQuery query(m_database);
    bool success = true;

    // detector table
    query.prepare("CREATE TABLE IF NOT EXISTS detector ("
                  "manufacturer TEXT, "
                  "instrumentModel TEXT NOT NULL, "
                  "serialNumber TEXT, "
                  "detectorType TEXT, "
                  "probeType TEXT, "
                  "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "detectorCode INTEGER NOT NULL DEFAULT 12, "
                  "crystalType INTEGER NOT NULL DEFAULT 1)");
    success &= executeQuery(query, "Creating detector table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    // background table
    query.prepare("CREATE TABLE IF NOT EXISTS background ("
                  "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "spectrum TEXT NOT NULL, "
                  "acqTime INTEGER NOT NULL, "
                  "realTime REAL NOT NULL, "
                  "detectorId INTEGER, "
                  "date TEXT)");
    success &= executeQuery(query, "Creating background table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    // calibration table
    query.prepare("CREATE TABLE IF NOT EXISTS calibration ("
                  "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "detector_id INTEGER NOT NULL DEFAULT -1, "
                  "coef_a INTEGER NOT NULL, "
                  "coef_b INTEGER NOT NULL, "
                  "coef_c INTEGER NOT NULL, "
                  "gc INTEGER, "
                  "ratio REAL NOT NULL DEFAULT 1.0, "
                  "spectrum TEXT, " // For debugging, not loaded into model
                  "chpeak_a REAL, "
                  "chpeak_b REAL, "
                  "chpeak_c REAL, "
                  "time TEXT, "
                  "temperature REAL)");
    success &= executeQuery(query, "Creating calibration table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    // event table
    query.prepare("CREATE TABLE IF NOT EXISTS event ("
                  "event_id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "softwareVersion TEXT NOT NULL DEFAULT '1.0.0', "
                  "dateBegin TEXT, "
                  "dateFinish TEXT, "
                  "liveTime REAL, "
                  "realTime REAL, "
                  "avgGamma_nSv REAL, "
                  "maxGamma_nSv REAL, "
                  "minGamma_nSv REAL, "
                  "avgFillCps REAL, "
                  "detectorId INTEGER, "
                  "detail_id INTEGER, "
                  "background_id INTEGER, "
                  "calibration_id INTEGER, "
                  "avgCps REAL NOT NULL DEFAULT 0, "
                  "maxCps REAL NOT NULL DEFAULT 0, "
                  "minCps REAL NOT NULL DEFAULT 0, "
                  "e1Energy NUMERIC NOT NULL DEFAULT 0, "
                  "e1Branching NUMERIC NOT NULL DEFAULT 0, "
                  "e1Netcount NUMERIC NOT NULL DEFAULT 0, "
                  "e2Energy NUMERIC NOT NULL DEFAULT 0, "
                  "e2Branching NUMERIC NOT NULL DEFAULT 0, "
                  "e2Netcount NUMERIC NOT NULL DEFAULT 0, "
                  "PipeMaterial TEXT, "
                  "PipeThickness NUMERIC NOT NULL DEFAULT 0, "
                  "PipeDiameter NUMERIC NOT NULL DEFAULT 0, "
                  "ClogMaterial TEXT, "
                  "ClogDensity NUMERIC NOT NULL DEFAULT 0, "
                  "ClogThickness NUMERIC NOT NULL DEFAULT 0, "
                  "ClogRatio NUMERIC NOT NULL DEFAULT 0)");
    success &= executeQuery(query, "Creating event table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    // event_detail table (for lazy load)
    query.prepare("CREATE TABLE IF NOT EXISTS event_detail ("
                  "id INTEGER NOT NULL PRIMARY KEY AUTOINCREMENT, "
                  "spectrum TEXT, "
                  "event_id INTEGER, "
                  "FOREIGN KEY(event_id) REFERENCES event(event_id) ON DELETE CASCADE ON UPDATE CASCADE)");
    success &= executeQuery(query, "Creating event_detail table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    // setup table
    query.prepare("CREATE TABLE IF NOT EXISTS setup ("
                  "Name TEXT NOT NULL UNIQUE, "
                  "Value TEXT, "
                  "PRIMARY KEY(Name))");
    success &= executeQuery(query, "Creating setup table");
    // Redundant log removed: if (!success) logE() << query.lastError().text();

    return success;
}

std::shared_ptr<DetectorInfo> DatabaseManager::getDetectorById(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, manufacturer, instrumentModel, serialNumber, detectorType, probeType, detectorCode, crystalType FROM detector WHERE id = :id");
    query.bindValue(":id", id);

    if (!executeQuery(query, "Fetching detector by ID")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;    }

    if (query.next()) {
        auto detector = std::make_shared<DetectorInfo>();
        detector->id = query.value("id").toInt();
        detector->manufacture = query.value("manufacturer").toString();
        detector->model = query.value("instrumentModel").toString(); // Maps instrumentModel to model
        detector->serialNumber = query.value("serialNumber").toString();
        detector->detectorType = query.value("detectorType").toString();
        detector->probeType = query.value("probeType").toString();
        detector->detectorCode = std::make_shared<DetectorCode>(static_cast<DetectorCode_E>(query.value("detectorCode").toInt()));
        return detector;
    }

    return nullptr;
}

void DatabaseManager::loadEventDetailsInto(Event* event)
{
    if (!event) {
        logE() << "Attempted to load details into a null Event pointer.";
        return;
    }

    long detailId = event->getDetailId();
    if (detailId <= 0) {
        logW() << "Event with ID" << event->getId() << "has no valid detail_id. No spectrum to load.";
        return;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT spectrum FROM event_detail WHERE id = :detail_id");
    query.bindValue(":detail_id", static_cast<qlonglong>(detailId));

    if (!executeQuery(query, "Fetching event spectrum from event_detail")) {
        // Redundant log removed. Original: logE() << "Failed to query event_detail for event ID" << event->getId() << ":" << query.lastError().text();
        return;
    }

    if (query.next()) {
        QString spectrumDataStr = query.value("spectrum").toString();
        if (!spectrumDataStr.isEmpty()) {

            std::shared_ptr<Spectrum> spc = std::shared_ptr<Spectrum>(Spectrum::pFromString(spectrumDataStr));
            event->setSpectrum(spc);
        } else {
            logW() << "Spectrum data string was empty or invalid for Event ID:" << event->getId();
        }
    }
}

std::shared_ptr<DetectorInfo> DatabaseManager::getDetectorByCriteria(
    const QString &serialNumber,
    const QString &model,
    const QString &probeType,
    const QString &detectorCodeStr,
    const QString &crystalTypeStr)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, manufacturer, instrumentModel, serialNumber, detectorType, probeType, detectorCode, crystalType FROM detector WHERE "
                 "serialNumber = :serialNumber AND "
                 "instrumentModel = :instrumentModel AND "
                 "probeType = :probeType AND "
                 "detectorCode = :detectorCode AND "
                 "crystalType = :crystalType");

    DetectorCode_E detCodeEnum = static_cast<DetectorCode_E>(detectorCodeStr.toInt());
    CrystalType crystalTypeEnum = static_cast<CrystalType>(crystalTypeStr.toInt());

    query.bindValue(":serialNumber", serialNumber);
    query.bindValue(":instrumentModel", model);
    query.bindValue(":probeType", probeType);
    query.bindValue(":detectorCode", static_cast<int>(detCodeEnum));
    query.bindValue(":crystalType", static_cast<int>(crystalTypeEnum));

    if (!executeQuery(query, "Fetching detector by criteria")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        auto detector = std::make_shared<DetectorInfo>();
        detector->id = query.value("id").toInt();
        detector->manufacture = query.value("manufacturer").toString();
        detector->model = query.value("instrumentModel").toString();
        detector->serialNumber = query.value("serialNumber").toString();
        detector->detectorType = query.value("detectorType").toString();
        detector->probeType = query.value("probeType").toString();
        detector->detectorCode = std::make_shared<DetectorCode>(static_cast<DetectorCode_E>(query.value("detectorCode").toInt()));
        return detector;
    }

    // If not found, insert new detector
    query.prepare("INSERT INTO detector (manufacturer, instrumentModel, serialNumber, detectorType, probeType, detectorCode, crystalType) "
                 "VALUES (:manufacturer, :instrumentModel, :serialNumber, :detectorType, :probeType, :detectorCode, :crystalType)");
    query.bindValue(":manufacturer", ""); // Manufacturer is optional in schema
    query.bindValue(":instrumentModel", model);
    query.bindValue(":serialNumber", serialNumber);
    query.bindValue(":detectorType", ""); // DetectorType is optional
    query.bindValue(":probeType", probeType);
    query.bindValue(":detectorCode", static_cast<int>(detCodeEnum));
    query.bindValue(":crystalType", static_cast<int>(crystalTypeEnum));

    if (!executeQuery(query, "Inserting new detector after criteria search failed")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    // Retrieve the newly inserted detector by its ID (lastInsertId)
    // Note: getDetectorById returns DetectorInfo, so we need to call it.
    return getDetectorById(query.lastInsertId().toInt());
}

std::shared_ptr<DetectorInfo> DatabaseManager::getLastDetector()
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, manufacturer, instrumentModel, serialNumber, detectorType, probeType, detectorCode, crystalType FROM detector WHERE "
                 "probeType = 'None' AND "
                 "detectorCode != :gp_code AND "
                 "serialNumber != '' "
                 "ORDER BY id DESC LIMIT 1"); // Order by id DESC as created_at is not in schema
    query.bindValue(":gp_code", static_cast<int>(NaI_2x2_GP));

    if (!executeQuery(query, "Fetching last detector")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        auto detector = std::make_shared<DetectorInfo>();
        detector->id = query.value("id").toInt();
        detector->manufacture = query.value("manufacturer").toString();
        detector->model = query.value("instrumentModel").toString();
        detector->serialNumber = query.value("serialNumber").toString();
        detector->detectorType = query.value("detectorType").toString();
        detector->probeType = query.value("probeType").toString();
        detector->detectorCode = std::make_shared<DetectorCode>(static_cast<DetectorCode_E>(query.value("detectorCode").toInt()));
        return detector;
    }

    return nullptr;
}

std::shared_ptr<Background> DatabaseManager::getBackgroundById(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, spectrum, acqTime, realTime, detectorId, date FROM background WHERE id = :id");
    query.bindValue(":id", id);

    if (!executeQuery(query, "Fetching background by ID")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        auto background = std::make_shared<Background>();
        background->id = query.value("id").toInt();

        QString spectrumDataStr = query.value("spectrum").toString();
        auto spectrum = std::shared_ptr<Spectrum>(Spectrum::pFromString(spectrumDataStr));

        spectrum->setAcqTime(query.value("acqTime").toInt()); // acqTime is INTEGER in schema
        spectrum->setRealTime(query.value("realTime").toDouble());
        spectrum->setDetectorID(query.value("detectorId").toInt());

        background->spc = spectrum;
        background->date = query.value("date").toString();
        return background;
    }

    return nullptr;
}

std::shared_ptr<Calibration> DatabaseManager::getCalibrationById(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT id, detector_id, coef_a, coef_b, coef_c, gc, ratio, chpeak_a, chpeak_b, chpeak_c, time, temperature FROM calibration WHERE id = :id");
    query.bindValue(":id", id);

    if (!executeQuery(query, "Fetching calibration by ID")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        auto calibration = std::make_shared<Calibration>();
        calibration->setId(query.value("id").toInt());
        calibration->setDetectorId(query.value("detector_id").toInt());

        Coeffcients coefficients;
        coefficients[0] = query.value("coef_a").toDouble(); // Cast to double for model
        coefficients[1] = query.value("coef_b").toDouble();
        coefficients[2] = query.value("coef_c").toDouble();
        calibration->setCoefficients(coefficients);

        calibration->setGC(query.value("gc").toInt());
        calibration->setRatio(query.value("ratio").toDouble());

        // No spectrum loading for calibration as per user feedback
        // QByteArray spectrumData = query.value("spectrum").toByteArray(); // This column exists but is not loaded

        Coeffcients chPeaks; // This maps to chpeak_a,b,c
        chPeaks[0] = query.value("chpeak_a").toDouble();
        chPeaks[1] = query.value("chpeak_b").toDouble();
        chPeaks[2] = query.value("chpeak_c").toDouble();
        calibration->setChCoefficients(chPeaks); // Assuming chCoefficients maps to chpeak_a,b,c

        calibration->setDate(nucare::Timestamp::fromString(query.value("time").toString(), Qt::ISODate)); // 'time' in schema maps to 'date' in model
        calibration->setTemperature(query.value("temperature").toDouble());

        return calibration;
    }

    return nullptr;
}

QVector<std::shared_ptr<Event>> DatabaseManager::getEvents(int page, int pageSize)
{
    QVector<std::shared_ptr<Event>> events;
    QSqlQuery query(m_database);
    query.prepare("SELECT event_id, softwareVersion, dateBegin, dateFinish, liveTime, realTime, avgGamma_nSv, maxGamma_nSv, minGamma_nSv, avgFillCps, detectorId, detail_id, background_id, calibration_id, avgCps, maxCps, minCps, e1Energy, e1Branching, e1Netcount, e2Energy, e2Branching, e2Netcount, PipeMaterial, PipeThickness, PipeDiameter, ClogMaterial, ClogDensity, ClogThickness, ClogRatio FROM event ORDER BY event_id DESC LIMIT :limit OFFSET :offset");
    query.bindValue(":limit", pageSize);
    query.bindValue(":offset", page * pageSize);

    if (!executeQuery(query, QString::asprintf("Fetching events with pagination at %d", page))) {
        // Redundant log removed: logE() << query.lastError().text();
        return events;
    }

    while (query.next()) {
        auto event = std::make_shared<Event>();
        event->setId(query.value("event_id").toLongLong());
        event->setSoftwareVersion(query.value("softwareVersion").toString());
        event->setStartedTime(query.value("dateBegin").toDateTime());
        event->setFinishedTime(query.value("dateFinish").toDateTime());
        event->setLiveTime(query.value("liveTime").toDouble());
        event->setRealTime(query.value("realTime").toDouble());
        event->setAvgGamma_nSv(query.value("avgGamma_nSv").toDouble());
        event->setMaxGamma_nSv(query.value("maxGamma_nSv").toDouble());
        event->setMinGamma_nSv(query.value("minGamma_nSv").toDouble());
        event->setAvgFillCps(query.value("avgFillCps").toDouble());
        event->setDetectorId(query.value("detectorId").toLongLong());
        event->setDetailId(query.value("detail_id").toLongLong());
        event->setBackgroundId(query.value("background_id").toLongLong());
        event->setCalibrationId(query.value("calibration_id").toLongLong());
        event->setAvgCps(query.value("avgCps").toDouble());
        event->setMaxCps(query.value("maxCps").toDouble());
        event->setMinCps(query.value("minCps").toDouble());
        event->setE1Energy(query.value("e1Energy").toDouble());
        event->setE1Branching(query.value("e1Branching").toDouble());
        event->setE1Netcount(query.value("e1Netcount").toDouble());
        event->setE2Energy(query.value("e2Energy").toDouble());
        event->setE2Branching(query.value("e2Branching").toDouble());
        event->setE2Netcount(query.value("e2Netcount").toDouble());
        event->setPipeMaterial(query.value("PipeMaterial").toString());
        event->setPipeThickness(query.value("PipeThickness").toDouble());
        event->setPipeDiameter(query.value("PipeDiameter").toDouble());
        event->setClogMaterial(query.value("ClogMaterial").toString());
        event->setClogDensity(query.value("ClogDensity").toDouble());
        event->setClogThickness(query.value("ClogThickness").toDouble());
        event->setClogRatio(query.value("ClogRatio").toDouble());
        events.push_back(event);
    }

    return events;
}

std::shared_ptr<Event> DatabaseManager::getEventDetails(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT event_id, softwareVersion, dateBegin, dateFinish, liveTime, realTime, avgGamma_nSv, maxGamma_nSv, minGamma_nSv, avgFillCps, detectorId, detail_id, background_id, calibration_id, avgCps, maxCps, minCps, e1Energy, e1Branching, e1Netcount, e2Energy, e2Branching, e2Netcount, PipeMaterial, PipeThickness, PipeDiameter, ClogMaterial, ClogDensity, ClogThickness, ClogRatio FROM event WHERE event_id = :id");
    query.bindValue(":id", id);

    if (!executeQuery(query, "Fetching event details by ID")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {
        auto event = std::make_shared<Event>();
        event->setId(query.value("event_id").toLongLong());
        event->setSoftwareVersion(query.value("softwareVersion").toString());
        event->setStartedTime(query.value("dateBegin").toDateTime());
        event->setFinishedTime(query.value("dateFinish").toDateTime());
        event->setLiveTime(query.value("liveTime").toDouble());
        event->setRealTime(query.value("realTime").toDouble());
        event->setAvgGamma_nSv(query.value("avgGamma_nSv").toDouble());
        event->setMaxGamma_nSv(query.value("maxGamma_nSv").toDouble());
        event->setMinGamma_nSv(query.value("minGamma_nSv").toDouble());
        event->setAvgFillCps(query.value("avgFillCps").toDouble());
        event->setDetectorId(query.value("detectorId").toLongLong());
        event->setDetailId(query.value("detail_id").toLongLong());
        event->setBackgroundId(query.value("background_id").toLongLong());
        event->setCalibrationId(query.value("calibration_id").toLongLong());
        event->setAvgCps(query.value("avgCps").toDouble());
        event->setMaxCps(query.value("maxCps").toDouble());
        event->setMinCps(query.value("minCps").toDouble());
        event->setE1Energy(query.value("e1Energy").toDouble());
        event->setE1Branching(query.value("e1Branching").toDouble());
        event->setE1Netcount(query.value("e1Netcount").toDouble());
        event->setE2Energy(query.value("e2Energy").toDouble());
        event->setE2Branching(query.value("e2Branching").toDouble());
        event->setE2Netcount(query.value("e2Netcount").toDouble());
        event->setPipeMaterial(query.value("PipeMaterial").toString());
        event->setPipeThickness(query.value("PipeThickness").toDouble());
        event->setPipeDiameter(query.value("PipeDiameter").toDouble());
        event->setClogMaterial(query.value("ClogMaterial").toString());
        event->setClogDensity(query.value("ClogDensity").toDouble());
        event->setClogThickness(query.value("ClogThickness").toDouble());
        event->setClogRatio(query.value("ClogRatio").toDouble());
        return event;
    }

    return nullptr;
}

std::shared_ptr<DetectorCalibConfig> DatabaseManager::getDefaultDetectorConfig(const int detId)
{
    QString cmd = "SELECT * FROM detector_config WHERE detectorId = :detId OR detectorId = 0 ORDER BY id LIMIT 1";
    QSqlQuery query(m_database);
    query.prepare(cmd);
    query.bindValue(":detId", detId);
    if (!executeQuery(query, "Fetching default detector config")) {
        // Redundant log removed: logE() << query.lastError().text();
        return nullptr;
    }

    if (query.next()) {

        auto pRet = std::make_shared<DetectorCalibConfig>();
        pRet->detectorId = detId;
        if (!query.next()) {
            pRet->calib = {13, 372, 860};
            return pRet;
        }

        pRet->calib = {
            query.value("chPeakA").toDouble(),
            query.value("chPeakB").toDouble(),
            query.value("chPeakC").toDouble()
        };

        return pRet;
    }

    return nullptr;
}

// Insert functions
int DatabaseManager::insertDetector(const DetectorInfo* detector)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO detector (manufacturer, instrumentModel, serialNumber, detectorType, probeType, detectorCode, crystalType) "
                 "VALUES (:manufacturer, :instrumentModel, :serialNumber, :detectorType, :probeType, :detectorCode, :crystalType)");
    query.bindValue(":manufacturer", detector->manufacture);
    query.bindValue(":instrumentModel", detector->model);
    query.bindValue(":serialNumber", detector->serialNumber);
    query.bindValue(":detectorType", detector->detectorType);
    query.bindValue(":probeType", detector->probeType);
    query.bindValue(":detectorCode", static_cast<int>(detector->detectorCode->code));
    query.bindValue(":crystalType", static_cast<int>(detector->detectorCode->cType));

    if (!executeQuery(query, "Inserting new detector")) {
        // Redundant log removed: logE() << query.lastError().text();
        return -1; // Indicate failure
    }
    return query.lastInsertId().toInt();
}

int DatabaseManager::insertBackground(const Background* background)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO background (spectrum, acqTime, realTime, detectorId, date) "
                 "VALUES (:spectrum, :acqTime, :realTime, :detectorId, :date)");

    QByteArray spectrumData;
    if (background->spc) {
        spectrumData = background->spc->toString().toUtf8(); // Use Spectrum::toString()
    }

    query.bindValue(":spectrum", spectrumData);
    query.bindValue(":acqTime", background->spc ? static_cast<int>(background->spc->getAcqTime()) : 0); // acqTime is INTEGER
    query.bindValue(":realTime", background->spc ? background->spc->getRealTime() : 0.0);
    query.bindValue(":detectorId", background->spc ? background->spc->getDetectorID() : -1);
    query.bindValue(":date", background->date);

    if (!executeQuery(query, "Inserting new background")) {
        // Redundant log removed: logE() << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

int DatabaseManager::insertCalibration(const Calibration* calibration)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO calibration (detector_id, coef_a, coef_b, coef_c, gc, ratio, chpeak_a, chpeak_b, chpeak_c, time, temperature) "
                 "VALUES (:detector_id, :coef_a, :coef_b, :coef_c, :gc, :ratio, :chpeak_a, :chpeak_b, :chpeak_c, :time, :temperature)");

    query.bindValue(":detector_id", calibration->getDetectorId());
    query.bindValue(":coef_a", static_cast<int>(calibration->coefficients()[0])); // Cast to INTEGER
    query.bindValue(":coef_b", static_cast<int>(calibration->coefficients()[1]));
    query.bindValue(":coef_c", static_cast<int>(calibration->coefficients()[2]));
    query.bindValue(":gc", calibration->getGC());
    query.bindValue(":ratio", calibration->getRatio());
    query.bindValue(":chpeak_a", calibration->chCoefficients()[0]); // Assuming chCoefficients maps to chpeak_a,b,c
    query.bindValue(":chpeak_b", calibration->chCoefficients()[1]);
    query.bindValue(":chpeak_c", calibration->chCoefficients()[2]);
    query.bindValue(":time", calibration->getDate().toString(Qt::ISODate)); // 'time' in schema maps to 'date' in model
    query.bindValue(":temperature", calibration->temperature());

    // No spectrum binding for calibration as per user feedback
    // query.bindValue(":spectrum", spectrumData);

    if (!executeQuery(query, "Inserting new calibration")) {
        // Redundant log removed: logE() << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt();
}

qlonglong DatabaseManager::insertEvent(const Event* event)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO event (softwareVersion, dateBegin, dateFinish, liveTime, realTime, avgGamma_nSv, maxGamma_nSv, minGamma_nSv, avgFillCps, detectorId, detail_id, background_id, calibration_id, avgCps, maxCps, minCps, e1Energy, e1Branching, e1Netcount, e2Energy, e2Branching, e2Netcount, PipeMaterial, PipeThickness, PipeDiameter, ClogMaterial, ClogDensity, ClogThickness, ClogRatio) "
                 "VALUES (:softwareVersion, :dateBegin, :dateFinish, :liveTime, :realTime, :avgGamma_nSv, :maxGamma_nSv, :minGamma_nSv, :avgFillCps, :detectorId, :detail_id, :background_id, :calibration_id, :avgCps, :maxCps, :minCps, :e1Energy, :e1Branching, :e1Netcount, :e2Energy, :e2Branching, :e2Netcount, :PipeMaterial, :PipeThickness, :PipeDiameter, :ClogMaterial, :ClogDensity, :ClogThickness, :ClogRatio)");

    query.bindValue(":softwareVersion", event->getSoftwareVersion());
    query.bindValue(":dateBegin", event->getStartedTime());
    query.bindValue(":dateFinish", event->getFinishedTime());
    query.bindValue(":liveTime", event->getLiveTime());
    query.bindValue(":realTime", event->getRealTime());
    query.bindValue(":avgGamma_nSv", event->getAvgGamma_nSv());
    query.bindValue(":maxGamma_nSv", event->getMaxGamma_nSv());
    query.bindValue(":minGamma_nSv", event->getMinGamma_nSv());
    query.bindValue(":avgFillCps", event->getAvgFillCps());
    query.bindValue(":detectorId", static_cast<qlonglong>(event->getDetectorId()));
    query.bindValue(":detail_id", static_cast<qlonglong>(event->getDetailId()));
    query.bindValue(":background_id", static_cast<qlonglong>(event->getBackgroundId()));
    query.bindValue(":calibration_id", static_cast<qlonglong>(event->getCalibrationId()));
    query.bindValue(":avgCps", event->getAvgCps());
    query.bindValue(":maxCps", event->getMaxCps());
    query.bindValue(":minCps", event->getMinCps());
    query.bindValue(":e1Energy", event->getE1Energy());
    query.bindValue(":e1Branching", event->getE1Branching());
    query.bindValue(":e1Netcount", event->getE1Netcount());
    query.bindValue(":e2Energy", event->getE2Energy());
    query.bindValue(":e2Branching", event->getE2Branching());
    query.bindValue(":e2Netcount", event->getE2Netcount());
    query.bindValue(":PipeMaterial", event->getPipeMaterial());
    query.bindValue(":PipeThickness", event->getPipeThickness());
    query.bindValue(":PipeDiameter", event->getPipeDiameter());
    query.bindValue(":ClogMaterial", event->getClogMaterial());
    query.bindValue(":ClogDensity", event->getClogDensity());
    query.bindValue(":ClogThickness", event->getClogThickness());
    query.bindValue(":ClogRatio", event->getClogRatio());

    if (!executeQuery(query, "Inserting new event")) {
        // Redundant log removed: logE() << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

qlonglong DatabaseManager::insertEventDetail(qlonglong eventId, const QString& spectrumData)
{
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO event_detail (event_id, spectrum) VALUES (:event_id, :spectrum)");
    query.bindValue(":event_id", eventId);
    query.bindValue(":spectrum", spectrumData);

    if (!executeQuery(query, "Inserting event detail")) {
        // executeQuery already logs the error
        return -1;
    }
    return query.lastInsertId().toLongLong();
}

int DatabaseManager::insertDetectorCalibConfig(const DetectorCalibConfig* config)
{
    QSqlQuery query(m_database);
    // Use INSERT OR REPLACE to update if detectorId exists, otherwise insert
    query.prepare("INSERT OR REPLACE INTO detector_config (detectorId, chPeakA, chPeakB, chPeakC, Time, Spectrum) "
                 "VALUES (:detectorId, :chPeakA, :chPeakB, :chPeakC, :Time, :Spectrum)");

    query.bindValue(":detectorId", config->detectorId);
    query.bindValue(":chPeakA", config->calib[0]);
    query.bindValue(":chPeakB", config->calib[1]);
    query.bindValue(":chPeakC", config->calib[2]);
    query.bindValue(":Time", config->time.toString(Qt::ISODate));

    QByteArray spectrumData;
    if (config->spc) {
        spectrumData = config->spc->toString().toUtf8();
    }
    query.bindValue(":Spectrum", spectrumData);

    if (!executeQuery(query, "Inserting or replacing detector calib config")) {
        // Redundant log removed. Original: logE() << "Failed to insert/replace DetectorCalibConfig:" << query.lastError().text();
        return -1;
    }
    return query.lastInsertId().toInt(); // Returns the rowid of the last inserted row
}

QSqlQuery DatabaseManager::executeSingleRowQuery(const QString& queryString, const QVariantMap& bindValues)
{
    QSqlQuery query(m_database);
    query.prepare(queryString);
    for (auto it = bindValues.constBegin(); it != bindValues.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!executeQuery(query, queryString)) { // Using queryString as context, can be improved
        // Redundant log removed. Original: logE() << "SQL Query failed:" << query.lastError().text() << "Query:" << queryString;
        // executeQuery already logs the error, the context (which is the queryString here), and the bound query.
    }
    return query;
}

std::shared_ptr<Background> DatabaseManager::getLatestBackground(int detectorId)
{
    QString queryString = "SELECT id, spectrum, acqTime, realTime, detectorId, date FROM background WHERE detectorId = :detectorId ORDER BY date DESC LIMIT 1";
    QVariantMap bindValues;
    bindValues[":detectorId"] = detectorId;

    QSqlQuery query = executeSingleRowQuery(queryString, bindValues); // This call doesn't need to change if executeSingleRowQuery handles executeQuery

    if (query.next()) {
        auto background = std::make_shared<Background>();
        background->id = query.value("id").toInt();

        QString spectrumDataStr = query.value("spectrum").toString();
        auto spectrum = std::shared_ptr<Spectrum>(Spectrum::pFromString(spectrumDataStr));

        spectrum->setAcqTime(query.value("acqTime").toInt());
        spectrum->setRealTime(query.value("realTime").toDouble());
        spectrum->setDetectorID(query.value("detectorId").toInt());

        background->spc = spectrum;
        background->date = query.value("date").toString();
        return background;
    }

    return nullptr;
}

std::shared_ptr<Calibration> DatabaseManager::getLatestCalibration(int detectorId)
{
    QString queryString = "SELECT id, detector_id, coef_a, coef_b, coef_c, gc, ratio, chpeak_a, chpeak_b, chpeak_c, time, temperature FROM calibration WHERE detector_id = :detector_id ORDER BY time DESC LIMIT 1";
    QVariantMap bindValues;
    bindValues[":detector_id"] = detectorId;

    QSqlQuery query = executeSingleRowQuery(queryString, bindValues);

    if (query.next()) {
        auto calibration = std::make_shared<Calibration>();
        calibration->setId(query.value("id").toInt());
        calibration->setDetectorId(query.value("detector_id").toInt());

        Coeffcients coefficients;
        coefficients[0] = query.value("coef_a").toDouble();
        coefficients[1] = query.value("coef_b").toDouble();
        coefficients[2] = query.value("coef_c").toDouble();
        calibration->setCoefficients(coefficients);

        calibration->setGC(query.value("gc").toInt());
        calibration->setRatio(query.value("ratio").toDouble());

        Coeffcients chPeaks;
        chPeaks[0] = query.value("chpeak_a").toDouble();
        chPeaks[1] = query.value("chpeak_b").toDouble();
        chPeaks[2] = query.value("chpeak_c").toDouble();
        calibration->setChCoefficients(chPeaks);

        calibration->setDate(nucare::Timestamp::fromString(query.value("time").toString(), Qt::ISODate));
        calibration->setTemperature(query.value("temperature").toDouble());

        return calibration;
    }

    return nullptr;
}

QVariant DatabaseManager::getSetting(const QString& name, const QVariant& defaultValue) {
    QSqlQuery query(m_database);
    query.prepare("SELECT Value FROM setup WHERE Name = ?");
    query.addBindValue(name);

    if (executeQuery(query, "Getting setting by name") && query.next()) {
        return query.value(0);
    }
    return defaultValue;
}

void DatabaseManager::setSetting(const QString& name, const QVariant& value) {
    QSqlQuery query(m_database);
    query.prepare("INSERT OR REPLACE INTO setup (Name, Value) VALUES (?, ?)");
    query.addBindValue(name);
    query.addBindValue(value.toString());

    if (!executeQuery(query, "Saving setting: " + name)) {
        // executeQuery logs the error, context ("Saving setting: [name]"), and the SQL.
    }
}

QMap<QString, QVariant> DatabaseManager::getAllSettings() {
    QMap<QString, QVariant> settings;
    QSqlQuery query(m_database); // Ensure query is associated with the database
    query.prepare("SELECT Name, Value FROM setup");

    if (executeQuery(query, "Getting all settings")) {
        while (query.next()) {
            settings.insert(query.value(0).toString(), query.value(1));
        }
    }

    return settings;
}

int DatabaseManager::getTotalEventCount()
{
    if (!m_database.isOpen()) {
        logE() << "Database not open when trying to get total event count.";
        return 0;
    }

    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM event");

    if (!executeQuery(query, "Getting total event count")) {
        return 0;
    }

    if (query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

} // namespace nucare
