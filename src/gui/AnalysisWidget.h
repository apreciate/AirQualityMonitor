#pragma once

#include <QWidget>
#include <QLabel>
#include <QTableWidget>
#include <QDateTimeEdit>
#include <QPushButton>
#include <QGroupBox>
#include "../models/Measurement.h"
#include "../analysis/DataAnalyzer.h"

/**
 * @brief Widget prezentujący wyniki analizy statystycznej pomiarów.
 *
 * Wyświetla: min, max, średnią, trend oraz szczegółową tabelę danych.
 */
class AnalysisWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalysisWidget(QWidget *parent = nullptr);

    /**
     * @brief Ustawia dane pomiarowe i przelicza analizę.
     * @param measurement Obiekt Measurement do analizy.
     */
    void setMeasurement(const Measurement &measurement);

private slots:
    void onAnalyzeRange();

private:
    void setupUi();
    void updateAnalysis(const Measurement &m);
    void setStatLabel(QLabel *label, const QString &value, const QString &color = "");

    // Statystyki
    QLabel *m_minLabel    = nullptr;
    QLabel *m_minDate     = nullptr;
    QLabel *m_maxLabel    = nullptr;
    QLabel *m_maxDate     = nullptr;
    QLabel *m_avgLabel    = nullptr;
    QLabel *m_trendLabel  = nullptr;
    QLabel *m_countLabel  = nullptr;

    // Zakres
    QDateTimeEdit *m_fromEdit  = nullptr;
    QDateTimeEdit *m_toEdit    = nullptr;
    QPushButton   *m_analyzeBtn = nullptr;

    // Tabela danych
    QTableWidget *m_table = nullptr;

    Measurement m_measurement;
};
