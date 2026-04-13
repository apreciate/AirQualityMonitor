#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <memory>
#include <vector>

#include "../api/GiosApiClient.h"
#include "../database/LocalDatabase.h"
#include "../models/Station.h"
#include "../models/Sensor.h"
#include "../models/Measurement.h"

class StationListWidget;
class ChartWidget;
class AnalysisWidget;
class MapWidget;

/**
 * @brief Główne okno aplikacji.
 *
 * Orkiestruje komunikację między widgetami, klientem API i bazą danych.
 * Zawiera zakładkowy interfejs z: listą stacji, wykresem, analizą i mapą.
 *
 * Wzorzec projektowy: Mediator — MainWindow pośredniczy między widgetami.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Opcjonalny rodzic Qt.
     */
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

private slots:
    /// @brief Obsługuje wybór stacji przez użytkownika.
    void onStationSelected(int stationId);

    /// @brief Obsługuje wybór sensora przez użytkownika.
    void onSensorSelected(int sensorId);

    /// @brief Obsługuje kliknięcie "Odśwież stacje".
    void onRefreshStations();

    /// @brief Obsługuje kliknięcie "Zapisz dane".
    void onSaveData();

    /// @brief Pokazuje/ukrywa pasek postępu.
    void onLoadingStarted();
    void onLoadingFinished();

    /// @brief Wyświetla komunikat o błędzie z propozycją użycia danych historycznych.
    void onApiError(const QString &message, std::function<void()> fallback);

private:
    void setupUi();
    void setupMenuBar();
    void setupStatusBar();
    void connectSignals();
    void loadStationsFromApi();
    void loadStationsFromDb();
    void showError(const QString &msg);
    void showInfo(const QString &msg);

    // Widgety
    QTabWidget       *m_tabs        = nullptr;
    StationListWidget *m_stationList = nullptr;
    ChartWidget      *m_chart       = nullptr;
    AnalysisWidget   *m_analysis    = nullptr;
    MapWidget        *m_map         = nullptr;

    // Status bar
    QProgressBar *m_progressBar = nullptr;
    QLabel       *m_statusLabel = nullptr;

    // Dane
    GiosApiClient         *m_apiClient  = nullptr;
    std::vector<Station>   m_stations;
    std::vector<Sensor>    m_sensors;
    Measurement            m_currentMeasurement;
    int                    m_currentStationId = -1;
    int                    m_currentSensorId  = -1;
};
