#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>
#include <vector>
#include <optional>

/**
 * @brief Pojedynczy punkt pomiarowy — data + wartość.
 *
 * Wartość jest opcjonalna (std::optional), ponieważ API może zwrócić null
 * gdy pomiar nie był w danej chwili dostępny.
 */
struct MeasurementPoint {
    QDateTime           date;   ///< Data i czas pomiaru
    std::optional<double> value; ///< Wartość pomiaru (nullopt jeśli brak danych)

    /**
     * @brief Tworzy punkt pomiarowy z pary JSON date/value.
     * @param obj Obiekt JSON z polami "date" i "value".
     * @return Wypełniony obiekt MeasurementPoint.
     */
    static MeasurementPoint fromJson(const QJsonObject &obj);

    /// @brief Serializuje punkt do JSON.
    QJsonObject toJson() const;
};

/**
 * @brief Zestaw pomiarów dla danego stanowiska pomiarowego.
 *
 * Przechowuje klucz parametru oraz listę punktów pomiarowych.
 */
struct Measurement {
    int                         sensorId;   ///< ID stanowiska pomiarowego
    QString                     key;        ///< Kod mierzonego parametru
    std::vector<MeasurementPoint> values;   ///< Lista punktów pomiarowych

    /**
     * @brief Tworzy obiekt Measurement z odpowiedzi API.
     * @param sensorId ID stanowiska pomiarowego.
     * @param json     Obiekt JSON zawierający "key" i "values".
     * @return Wypełniony obiekt Measurement.
     */
    static Measurement fromJson(int sensorId, const QJsonObject &json);

    /// @brief Serializuje do JSON (zapis do bazy lokalnej).
    QJsonObject toJson() const;

    /**
     * @brief Filtruje punkty pomiarowe do podanego zakresu czasowego.
     * @param from Początek zakresu.
     * @param to   Koniec zakresu.
     * @return Nowy obiekt Measurement z odfiltrowanymi danymi.
     */
    Measurement filtered(const QDateTime &from, const QDateTime &to) const;
};
