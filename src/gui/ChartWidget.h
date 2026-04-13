#pragma once

#include <QWidget>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "../models/Measurement.h"

/**
 * @brief Widget wyświetlający dane pomiarowe w formie wykresu liniowego.
 *
 * Umożliwia wybór zakresu czasowego i eksport wykresu do pliku PNG.
 * Używa biblioteki Qt Charts.
 */
class ChartWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ChartWidget(QWidget *parent = nullptr);

    /**
     * @brief Ustawia dane pomiarowe i przerysowuje wykres.
     * @param measurement Obiekt Measurement z danymi do wyświetlenia.
     */
    void setMeasurement(const Measurement &measurement);

private slots:
    void onRangePresetChanged(int index);
    void onCustomRangeApplied();
    void onExportChart();

private:
    void setupUi();
    void updateChart();
    void applyRange(const QDateTime &from, const QDateTime &to);

    // Widgety
    QChartView    *m_chartView   = nullptr;
    QDateTimeEdit *m_fromEdit    = nullptr;
    QDateTimeEdit *m_toEdit      = nullptr;
    QPushButton   *m_applyBtn    = nullptr;
    QPushButton   *m_exportBtn   = nullptr;
    QComboBox     *m_presetCombo = nullptr;
    QLabel        *m_infoLabel   = nullptr;

    // Dane
    Measurement m_measurement;
};
