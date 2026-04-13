#pragma once

#include <QString>
#include <QJsonDocument>
#include <vector>
#include <optional>

#include "../models/Station.h"
#include "../models/Sensor.h"
#include "../models/Measurement.h"
#include "../models/AirQualityIndex.h"

/**
 * @brief Lokalna baza danych oparta na plikach JSON.
 *
 * Wzorzec projektowy: Singleton — zapewnia jeden punkt dostępu do bazy danych.
 * Dane przechowywane są w katalogu użytkownika w folderze AirQualityMonitor/.
 *
 * Struktura plików:
 * - stations.json     — lista wszystkich pobranych stacji
 * - sensors_<id>.json — sensory dla stacji o danym id
 * - measurements_<sensorId>.json — dane pomiarowe dla sensora
 * - aqi_<stationId>.json — indeks jakości powietrza
 */
class LocalDatabase
{
public:
    /**
     * @brief Zwraca jedyną instancję klasy (Singleton).
     * @return Referencja do instancji LocalDatabase.
     */
    static LocalDatabase& instance();

    // Usunięcie kopiowania (Singleton)
    LocalDatabase(const LocalDatabase&) = delete;
    LocalDatabase& operator=(const LocalDatabase&) = delete;

    // ─── Stacje ───────────────────────────────────────────────────────────────

    /**
     * @brief Zapisuje listę stacji do lokalnej bazy.
     * @param stations Wektor obiektów Station.
     * @throws std::runtime_error jeśli zapis nie powiedzie się.
     */
    void saveStations(const std::vector<Station> &stations);

    /**
     * @brief Wczytuje stacje z lokalnej bazy.
     * @return Wektor stacji lub pusty wektor jeśli brak danych.
     */
    std::vector<Station> loadStations() const;

    // ─── Sensory ──────────────────────────────────────────────────────────────

    /**
     * @brief Zapisuje sensory dla danej stacji.
     * @param stationId ID stacji.
     * @param sensors   Wektor sensorów.
     */
    void saveSensors(int stationId, const std::vector<Sensor> &sensors);

    /**
     * @brief Wczytuje sensory dla danej stacji.
     * @param stationId ID stacji.
     * @return Wektor sensorów lub pusty wektor.
     */
    std::vector<Sensor> loadSensors(int stationId) const;

    // ─── Pomiary ──────────────────────────────────────────────────────────────

    /**
     * @brief Zapisuje dane pomiarowe dla danego sensora.
     * @param measurement Obiekt Measurement do zapisania.
     */
    void saveMeasurement(const Measurement &measurement);

    /**
     * @brief Wczytuje dane pomiarowe dla danego sensora.
     * @param sensorId ID stanowiska pomiarowego.
     * @return Opcjonalny obiekt Measurement (nullopt jeśli brak danych).
     */
    std::optional<Measurement> loadMeasurement(int sensorId) const;

    // ─── Indeks jakości powietrza ─────────────────────────────────────────────

    /**
     * @brief Zapisuje indeks jakości powietrza dla stacji.
     * @param aqi Obiekt AirQualityIndex.
     */
    void saveAirQualityIndex(const AirQualityIndex &aqi);

    /**
     * @brief Wczytuje indeks jakości dla stacji.
     * @param stationId ID stacji.
     * @return Opcjonalny AirQualityIndex.
     */
    std::optional<AirQualityIndex> loadAirQualityIndex(int stationId) const;

    /**
     * @brief Zwraca ścieżkę do katalogu z danymi.
     * @return Bezwzględna ścieżka do folderu danych.
     */
    QString dataDirectory() const;

private:
    LocalDatabase();  ///< Prywatny konstruktor (Singleton)

    QString m_dataDir; ///< Katalog przechowywania plików JSON

    /**
     * @brief Zapisuje dokument JSON do pliku.
     * @param filename Nazwa pliku (bez ścieżki).
     * @param doc      Dokument do zapisu.
     * @throws std::runtime_error przy błędzie zapisu.
     */
    void writeJson(const QString &filename, const QJsonDocument &doc) const;

    /**
     * @brief Wczytuje dokument JSON z pliku.
     * @param filename Nazwa pliku.
     * @return Opcjonalny dokument JSON (nullopt jeśli plik nie istnieje).
     */
    std::optional<QJsonDocument> readJson(const QString &filename) const;
};
