#pragma once

#include <QString>
#include <QJsonObject>

/**
 * @brief Reprezentacja stanowiska pomiarowego (czujnika) w stacji GIOŚ.
 *
 * Każda stacja może posiadać wiele stanowisk mierzących różne parametry,
 * np. PM10, PM2.5, NO2, SO2, CO, O3.
 */
struct Sensor {
    int     id;             ///< Unikalny identyfikator stanowiska
    int     stationId;      ///< ID stacji, do której należy stanowisko
    QString paramName;      ///< Pełna nazwa parametru (np. "pył zawieszony PM10")
    QString paramFormula;   ///< Symbol chemiczny / skrót (np. "PM10")
    QString paramCode;      ///< Kod parametru używany w API
    int     paramId;        ///< Identyfikator parametru

    /**
     * @brief Tworzy obiekt Sensor z danych JSON zwróconych przez API.
     * @param json Obiekt JSON z danymi stanowiska.
     * @return Wypełniony obiekt Sensor.
     */
    static Sensor fromJson(const QJsonObject &json);

    /**
     * @brief Serializuje obiekt do formatu JSON.
     * @return Obiekt QJsonObject reprezentujący stanowisko.
     */
    QJsonObject toJson() const;

    /**
     * @brief Zwraca czytelny opis stanowiska do wyświetlenia w liście.
     * @return Sformatowany string z ID i nazwą parametru.
     */
    QString displayName() const;
};
