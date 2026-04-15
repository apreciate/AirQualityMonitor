#include "GiosApiClient.h"
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonObject>
#include <QJsonArray>

GiosApiClient::GiosApiClient(QObject *parent)
    : QObject(parent)
    , m_manager(new QNetworkAccessManager(this))
{}

void GiosApiClient::get(const QString &endpoint,
                        std::function<void(QJsonDocument)> onSuccess,
                        ErrorCallback onError)
{
    emit loadingStarted();
    QUrl url(QString("%1%2").arg(BASE_URL, endpoint));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess, onError]() {
        emit loadingFinished();
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            onError(tr("Błąd sieci: %1").arg(reply->errorString()));
            return;
        }
        QByteArray data = reply->readAll();
        QJsonParseError parseError;
        QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
        if (parseError.error != QJsonParseError::NoError) {
            onError(tr("Błąd parsowania JSON: %1").arg(parseError.errorString()));
            return;
        }
        onSuccess(doc);
    });
}

static QJsonArray extractArray(const QJsonDocument &doc, const QString &key = "")
{
    if (doc.isArray()) return doc.array();
    QJsonObject obj = doc.object();
    if (!key.isEmpty() && obj.contains(key)) return obj[key].toArray();
    for (const QString &k : obj.keys())
        if (obj[k].isArray()) return obj[k].toArray();
    return QJsonArray();
}

// Pomocnicza metoda do pobierania wszystkich stron
void GiosApiClient::fetchAllPages(const QString &baseEndpoint,
                                   const QString &arrayKey,
                                   std::function<void(QJsonArray)> onComplete,
                                   ErrorCallback onError,
                                   int page,
                                   QJsonArray accumulated)
{
    QString endpoint = QString("%1?page=%2&size=100").arg(baseEndpoint).arg(page);
    get(endpoint, [this, baseEndpoint, arrayKey, onComplete, onError, page, accumulated](QJsonDocument doc) mutable {
        QJsonObject root = doc.object();
        QJsonArray arr = root[arrayKey].toArray();

        for (const QJsonValue &v : arr)
            accumulated.append(v);

        int totalPages = root["totalPages"].toInt(1);
        if (page + 1 < totalPages) {
            fetchAllPages(baseEndpoint, arrayKey, onComplete, onError, page + 1, accumulated);
        } else {
            onComplete(accumulated);
        }
    }, onError);
}

void GiosApiClient::fetchAllStations(StationsCallback onSuccess, ErrorCallback onError)
{
    fetchAllPages("/station/findAll", "Lista stacji pomiarowych",
        [onSuccess](QJsonArray arr) {
            std::vector<Station> stations;
            stations.reserve(static_cast<size_t>(arr.size()));
            for (const QJsonValue &v : arr)
                stations.push_back(Station::fromJson(v.toObject()));
            onSuccess(std::move(stations));
        }, onError);
}

void GiosApiClient::fetchSensors(int stationId, SensorsCallback onSuccess, ErrorCallback onError)
{
    get(QString("/station/sensors/%1").arg(stationId), [onSuccess](QJsonDocument doc) {
        std::vector<Sensor> sensors;
        QJsonObject root = doc.object();
        QJsonArray arr = root.contains("Lista stanowisk pomiarowych")
                       ? root["Lista stanowisk pomiarowych"].toArray()
                       : extractArray(doc);
        sensors.reserve(static_cast<size_t>(arr.size()));
        for (const QJsonValue &v : arr)
            sensors.push_back(Sensor::fromJson(v.toObject()));
        onSuccess(std::move(sensors));
    }, onError);
}

void GiosApiClient::fetchMeasurements(int sensorId,
                                      MeasurementCallback onSuccess,
                                      ErrorCallback onError)
{
    fetchAllPages(QString("/data/getData/%1").arg(sensorId),
                  "Lista danych pomiarowych",
                  [onSuccess, sensorId](QJsonArray arr) {
                      Measurement m;
                      m.sensorId = sensorId;
                      // Klucz parametru z pierwszego elementu
                      if (!arr.isEmpty()) {
                          QJsonObject first = arr.first().toObject();
                          m.key = first["Wskaźnik - kod"].toString();
                          if (m.key.isEmpty()) m.key = first["Kod stanowiska"].toString();
                      }
                      for (const QJsonValue &v : arr)
                          m.values.push_back(MeasurementPoint::fromJson(v.toObject()));
                      onSuccess(std::move(m));
                  }, onError);
}


void GiosApiClient::fetchAirQualityIndex(int stationId,
                                          AqiCallback onSuccess,
                                          ErrorCallback onError)
{
    get(QString("/aqindex/getIndex/%1").arg(stationId),
        [onSuccess](QJsonDocument doc) {
            QJsonObject obj = doc.isObject() ? doc.object()
                            : doc.array().first().toObject();
            AirQualityIndex idx = AirQualityIndex::fromJson(obj);
            onSuccess(idx);
        }, onError);
}