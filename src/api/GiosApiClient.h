#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <functional>
#include <vector>

#include "../models/Station.h"
#include "../models/Sensor.h"
#include "../models/Measurement.h"
#include "../models/AirQualityIndex.h"

/**
 * @brief Klient REST API Głównego Inspektoratu Ochrony Środowiska.
 *
 * Klasa opakowuje wszystkie żądania HTTP do API GIOŚ (powietrze.gios.gov.pl).
 * Działa asynchronicznie — wyniki zwracane są przez callbacki lub sygnały Qt.
 *
 * Wzorzec projektowy: Facade — ukrywa szczegóły HTTP za prostym interfejsem.
 */
class GiosApiClient : public QObject
{
    Q_OBJECT

public:
    /// @brief Typ callbacku na sukces z listą stacji.
    using StationsCallback    = std::function<void(std::vector<Station>)>;
    /// @brief Typ callbacku na sukces z listą sensorów.
    using SensorsCallback     = std::function<void(std::vector<Sensor>)>;
    /// @brief Typ callbacku na sukces z danymi pomiarowymi.
    using MeasurementCallback = std::function<void(Measurement)>;
    /// @brief Typ callbacku na sukces z indeksem jakości.
    using AqiCallback         = std::function<void(AirQualityIndex)>;
    /// @brief Typ callbacku na błąd.
    using ErrorCallback       = std::function<void(QString)>;

    /**
     * @brief Konstruktor.
     * @param parent Opcjonalny rodzic Qt.
     */
    explicit GiosApiClient(QObject *parent = nullptr);

    /**
     * @brief Pobiera listę wszystkich stacji pomiarowych w Polsce.
     * @param onSuccess Callback wywoływany z listą stacji po sukcesie.
     * @param onError   Callback wywoływany z komunikatem błędu.
     */
    void fetchAllStations(StationsCallback onSuccess, ErrorCallback onError);

    /**
     * @brief Pobiera listę stanowisk pomiarowych dla danej stacji.
     * @param stationId ID stacji pomiarowej.
     * @param onSuccess Callback z listą sensorów.
     * @param onError   Callback z błędem.
     */
    void fetchSensors(int stationId, SensorsCallback onSuccess, ErrorCallback onError);

    /**
     * @brief Pobiera dane pomiarowe dla danego stanowiska.
     * @param sensorId  ID stanowiska pomiarowego.
     * @param onSuccess Callback z obiektem Measurement.
     * @param onError   Callback z błędem.
     */
    void fetchMeasurements(int sensorId, MeasurementCallback onSuccess, ErrorCallback onError);

    /**
     * @brief Pobiera indeks jakości powietrza dla danej stacji.
     * @param stationId ID stacji.
     * @param onSuccess Callback z indeksem jakości.
     * @param onError   Callback z błędem.
     */
    void fetchAirQualityIndex(int stationId, AqiCallback onSuccess, ErrorCallback onError);

signals:
    /// @brief Emitowany gdy trwa ładowanie danych z sieci.
    void loadingStarted();
    /// @brief Emitowany gdy ładowanie zakończone (sukces lub błąd).
    void loadingFinished();

private:
    static constexpr const char* BASE_URL = "https://api.gios.gov.pl/pjp-api/v1/rest";

    QNetworkAccessManager *m_manager; ///< Menedżer połączeń Qt Network

    /**
     * @brief Wykonuje asynchroniczne żądanie GET.
     * @param endpoint Ścieżka API (bez BASE_URL).
     * @param onSuccess Callback z odpowiedzią JSON.
     * @param onError   Callback z błędem.
     */
    void get(const QString &endpoint,
             std::function<void(QJsonDocument)> onSuccess,
             ErrorCallback onError);

    void fetchAllPages(const QString &baseEndpoint,
                       const QString &arrayKey,
                       std::function<void(QJsonArray)> onComplete,
                       ErrorCallback onError,
                       int page = 0,
                       QJsonArray accumulated = QJsonArray());
};
