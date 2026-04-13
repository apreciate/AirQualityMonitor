#include "LocalDatabase.h"
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <stdexcept>

// ─── Singleton ────────────────────────────────────────────────────────────────

LocalDatabase& LocalDatabase::instance()
{
    static LocalDatabase db;
    return db;
}

LocalDatabase::LocalDatabase()
{
    // Katalog danych: np. ~/.local/share/AirQualityMonitor/ (Linux)
    //                      %APPDATA%\AirQualityMonitor\    (Windows)
    QString base = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_dataDir = base + "/AirQualityMonitor";
    QDir().mkpath(m_dataDir);
}

QString LocalDatabase::dataDirectory() const
{
    return m_dataDir;
}

// ─── JSON I/O ─────────────────────────────────────────────────────────────────

void LocalDatabase::writeJson(const QString &filename, const QJsonDocument &doc) const
{
    QFile file(m_dataDir + "/" + filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        throw std::runtime_error(
            QString("Nie można zapisać pliku: %1").arg(file.fileName()).toStdString());

    file.write(doc.toJson(QJsonDocument::Indented));
}

std::optional<QJsonDocument> LocalDatabase::readJson(const QString &filename) const
{
    QFile file(m_dataDir + "/" + filename);
    if (!file.exists() || !file.open(QIODevice::ReadOnly))
        return std::nullopt;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError)
        return std::nullopt;

    return doc;
}

// ─── Stacje ───────────────────────────────────────────────────────────────────

void LocalDatabase::saveStations(const std::vector<Station> &stations)
{
    QJsonArray arr;
    for (const auto &s : stations)
        arr.append(s.toJson());
    writeJson("stations.json", QJsonDocument(arr));
}

std::vector<Station> LocalDatabase::loadStations() const
{
    auto doc = readJson("stations.json");
    if (!doc) return {};

    std::vector<Station> result;
    QJsonArray arr = doc->array();
    result.reserve(static_cast<size_t>(arr.size()));
    for (const auto &v : arr) {
        QJsonObject obj = v.toObject();
        Station s;
        s.id       = obj["id"].toInt();
        s.name     = obj["name"].toString();
        s.lat      = obj["lat"].toDouble();
        s.lon      = obj["lon"].toDouble();
        s.city     = obj["city"].toString();
        s.street   = obj["street"].toString();
        s.commune  = obj["commune"].toString();
        s.district = obj["district"].toString();
        s.province = obj["province"].toString();
        result.push_back(s);
    }
    return result;
}

// ─── Sensory ──────────────────────────────────────────────────────────────────

void LocalDatabase::saveSensors(int stationId, const std::vector<Sensor> &sensors)
{
    QJsonArray arr;
    for (const auto &s : sensors)
        arr.append(s.toJson());
    writeJson(QString("sensors_%1.json").arg(stationId), QJsonDocument(arr));
}

std::vector<Sensor> LocalDatabase::loadSensors(int stationId) const
{
    auto doc = readJson(QString("sensors_%1.json").arg(stationId));
    if (!doc) return {};

    std::vector<Sensor> result;
    QJsonArray arr = doc->array();
    result.reserve(static_cast<size_t>(arr.size()));
    for (const auto &v : arr) {
        QJsonObject obj = v.toObject();
        Sensor s;
        s.id          = obj["id"].toInt();
        s.stationId   = obj["stationId"].toInt();
        s.paramName   = obj["paramName"].toString();
        s.paramFormula= obj["paramFormula"].toString();
        s.paramCode   = obj["paramCode"].toString();
        s.paramId     = obj["paramId"].toInt();
        result.push_back(s);
    }
    return result;
}

// ─── Pomiary ──────────────────────────────────────────────────────────────────

void LocalDatabase::saveMeasurement(const Measurement &m)
{
    writeJson(QString("measurements_%1.json").arg(m.sensorId),
              QJsonDocument(m.toJson()));
}

std::optional<Measurement> LocalDatabase::loadMeasurement(int sensorId) const
{
    auto doc = readJson(QString("measurements_%1.json").arg(sensorId));
    if (!doc) return std::nullopt;
    return Measurement::fromJson(sensorId, doc->object());
}

// ─── Indeks jakości ───────────────────────────────────────────────────────────

void LocalDatabase::saveAirQualityIndex(const AirQualityIndex &aqi)
{
    writeJson(QString("aqi_%1.json").arg(aqi.stationId),
              QJsonDocument(aqi.toJson()));
}

std::optional<AirQualityIndex> LocalDatabase::loadAirQualityIndex(int stationId) const
{
    auto doc = readJson(QString("aqi_%1.json").arg(stationId));
    if (!doc) return std::nullopt;
    return AirQualityIndex::fromJson(doc->object());
}
