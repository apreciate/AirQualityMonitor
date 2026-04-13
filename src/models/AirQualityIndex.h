#pragma once

#include <QString>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief Poziom indeksu jakości powietrza.
 *
 * Mapuje wartość numeryczną (0–5) na czytelną nazwę i kolor.
 */
struct IndexLevel {
    int     id;             ///< Poziom od 0 (bardzo dobry) do 5 (bardzo zły)
    QString name;           ///< Tekstowy opis (np. "Bardzo dobry")

    /// @brief Kolor odpowiadający poziomowi indeksu (do GUI).
    QString color() const;

    /// @brief Tworzy IndexLevel z JSON.
    static IndexLevel fromJson(const QJsonObject &json);
};

/**
 * @brief Indeks jakości powietrza dla całej stacji pomiarowej.
 *
 * Zawiera ogólny indeks stacji oraz daty obliczenia/zebrania danych.
 */
struct AirQualityIndex {
    int         stationId;      ///< ID stacji
    QDateTime   calcDate;       ///< Data obliczenia indeksu
    QDateTime   sourceDate;     ///< Data zebrania danych źródłowych
    IndexLevel  stationIndex;   ///< Najgorszy indeks dla całej stacji

    /**
     * @brief Tworzy obiekt AirQualityIndex z danych JSON zwróconych przez API.
     * @param json Obiekt JSON z odpowiedzią API.
     * @return Wypełniony obiekt AirQualityIndex.
     */
    static AirQualityIndex fromJson(const QJsonObject &json);

    /// @brief Serializuje do JSON.
    QJsonObject toJson() const;
};
