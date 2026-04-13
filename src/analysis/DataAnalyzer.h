#pragma once

#include <optional>
#include <QDateTime>
#include "../models/Measurement.h"

/**
 * @brief Wyniki prostej analizy statystycznej danych pomiarowych.
 */
struct AnalysisResult {
    double    minValue;         ///< Wartość minimalna
    QDateTime minDate;          ///< Data wystąpienia minimum
    double    maxValue;         ///< Wartość maksymalna
    QDateTime maxDate;          ///< Data wystąpienia maksimum
    double    avgValue;         ///< Wartość średnia
    double    trend;            ///< Współczynnik trendu (>0 wzrost, <0 spadek)
    int       validCount;       ///< Liczba prawidłowych pomiarów (nie-null)
    int       totalCount;       ///< Łączna liczba punktów

    /**
     * @brief Zwraca tekstowy opis trendu.
     * @return "rosnący", "malejący" lub "stabilny".
     */
    QString trendDescription() const;
};

/**
 * @brief Klasa analizująca dane pomiarowe.
 *
 * Dostarcza metody obliczania statystyk: min, max, średnia, trend.
 * Wzorzec projektowy: Strategy — analiza może być rozszerzona bez modyfikacji GUI.
 */
class DataAnalyzer
{
public:
    /**
     * @brief Analizuje podany zestaw pomiarów.
     * @param measurement Obiekt Measurement do analizy.
     * @return Opcjonalny wynik analizy (nullopt jeśli brak danych).
     */
    static std::optional<AnalysisResult> analyze(const Measurement &measurement);

    /**
     * @brief Oblicza trend metodą regresji liniowej (metoda najmniejszych kwadratów).
     * @param values Lista wartości pomiarowych.
     * @return Współczynnik kierunkowy prostej regresji.
     */
    static double calculateTrend(const std::vector<double> &values);

    /**
     * @brief Filtruje punkty pomiarowe z wartościami null.
     * @param measurement Wejściowy obiekt Measurement.
     * @return Wektor par (data, wartość) bez null.
     */
    static std::vector<std::pair<QDateTime, double>>
        validPoints(const Measurement &measurement);
};
