#pragma once

#include <QString>
#include <QJsonObject>

/**
 * @brief Reprezentacja stacji pomiarowej GIOŚ.
 *
 * Przechowuje wszystkie informacje o stacji pomiarowej pobrane z API GIOŚ,
 * w tym lokalizację geograficzną i dane adresowe.
 */
struct Station {
    int     id;             ///< Unikalny identyfikator stacji
    QString name;           ///< Nazwa stacji pomiarowej
    double  lat;            ///< Szerokość geograficzna
    double  lon;            ///< Długość geograficzna
    QString city;           ///< Nazwa miejscowości
    QString street;         ///< Ulica
    QString commune;        ///< Gmina
    QString district;       ///< Powiat
    QString province;       ///< Województwo

    /**
     * @brief Tworzy obiekt Station z danych JSON zwróconych przez API.
     * @param json Obiekt JSON z danymi stacji.
     * @return Wypełniony obiekt Station.
     */
    static Station fromJson(const QJsonObject &json);

    /**
     * @brief Serializuje obiekt do formatu JSON (zapis do lokalnej bazy).
     * @return Obiekt QJsonObject reprezentujący stację.
     */
    QJsonObject toJson() const;

    /**
     * @brief Zwraca czytelny opis stacji do wyświetlenia w liście.
     * @return Sformatowany string z nazwą i miastem.
     */
    QString displayName() const;
};
