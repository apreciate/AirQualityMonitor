#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include "../models/Measurement.h"

/**
 * @brief Widget wykresu liniowego danych pomiarowych.
 *
 * Wyświetla pomiary z wybranego stanowiska w formie wykresu liniowego.
 * Obsługuje dwa tryby pracy:
 * - **tryb live** – dane z API, presety filtrują widok po czasie
 * - **tryb historyczny** – dane z lokalnej bazy, wykres jest zablokowany
 *   na zapisanym zakresie i nie dociąga aktualnych danych
 *
 * Dla szerszych zakresów czasowych stosowane jest próbkowanie (co N-ty punkt),
 * dzięki czemu wykresy 24h, tygodniowe i dwutygodniowe różnią się rozdzielczością.
 */
class ChartWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor.
     * @param parent Opcjonalny rodzic Qt.
     */
    explicit ChartWidget(QWidget *parent = nullptr);

    /**
     * @brief Ładuje dane w trybie live (pobrane z API).
     *
     * Odblokowuje presety i pozwala filtrować po czasie.
     * @param measurement Dane pomiarowe z API.
     */
    void setMeasurement(const Measurement &measurement);

    /**
     * @brief Ładuje dane w trybie historycznym (z lokalnej bazy).
     *
     * Blokuje presety i pola dat – wykres sztywno pokazuje zapisane dane.
     * @param measurement Dane pomiarowe z bazy lokalnej.
     */
    void setHistoricalMeasurement(const Measurement &measurement);

private slots:
    void onRangePresetChanged(int index);
    void onCustomRangeApplied();
    void onExportChart();

private:
    void setupUi();

    /**
     * @brief Rysuje wykres dla podanego zakresu z próbkowaniem.
     *
     * Próbkowanie: dla zakresu 24h – co 1 punkt, 3 dni – co 3, tydzień – co 7,
     * 2 tygodnie – co 14. Dzięki temu każdy przedział wygląda inaczej.
     * @param from Początek zakresu.
     * @param to   Koniec zakresu.
     */
    void applyRange(const QDateTime &from, const QDateTime &to);

    /**
     * @brief Blokuje lub odblokowuje kontrolki wyboru zakresu.
     * @param locked true = tryb historyczny (zablokowane).
     */
    void setControlsLocked(bool locked);

    QChartView    *m_chartView   = nullptr;
    QDateTimeEdit *m_fromEdit    = nullptr;
    QDateTimeEdit *m_toEdit      = nullptr;
    QPushButton   *m_applyBtn    = nullptr;
    QPushButton   *m_exportBtn   = nullptr;
    QComboBox     *m_presetCombo = nullptr;
    QLabel        *m_infoLabel   = nullptr;
    QLabel        *m_modeLabel   = nullptr;

    Measurement m_measurement;
    bool m_historyMode = false; ///< true gdy wyświetlamy dane historyczne
};
