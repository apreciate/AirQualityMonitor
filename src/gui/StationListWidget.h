#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <vector>

#include "../models/Station.h"
#include "../models/Sensor.h"
#include "../models/AirQualityIndex.h"

/**
 * @brief Widget do przeglądania i wyboru stacji i sensorów pomiarowych.
 *
 * Umożliwia filtrowanie stacji po mieście, wyświetla listę sensorów
 * oraz aktualny indeks jakości powietrza dla wybranej stacji.
 */
class StationListWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StationListWidget(QWidget *parent = nullptr);

    /**
     * @brief Ustawia listę stacji do wyświetlenia.
     * @param stations Wektor stacji.
     */
    void setStations(const std::vector<Station> &stations);

    /**
     * @brief Ustawia listę sensorów dla wybranej stacji.
     * @param sensors Wektor sensorów.
     */
    void setSensors(const std::vector<Sensor> &sensors);

    /**
     * @brief Ustawia indeks jakości powietrza dla wybranej stacji.
     * @param aqi Obiekt AirQualityIndex.
     */
    void setAirQualityIndex(const AirQualityIndex &aqi);

    /// @brief Czyści listę sensorów.
    void clearSensors();

signals:
    /// @brief Emitowany gdy użytkownik wybierze stację.
    void stationSelected(int stationId);
    /// @brief Emitowany gdy użytkownik wybierze sensor.
    void sensorSelected(int sensorId);
    /// @brief Emitowany gdy użytkownik kliknie "Odśwież".
    void refreshRequested();
    /// @brief Emitowany gdy użytkownik kliknie "Zapisz".
    void saveRequested();

private slots:
    void onStationClicked(QListWidgetItem *item);
    void onSensorClicked(QListWidgetItem *item);
    void onFilterChanged(const QString &text);
    void onProvinceChanged(const QString &province);

private:
    void setupUi();
    void refreshStationList();

    // Widgety
    QLineEdit    *m_filterEdit      = nullptr;
    QComboBox    *m_provinceCombo   = nullptr;
    QListWidget  *m_stationList     = nullptr;
    QListWidget  *m_sensorList      = nullptr;
    QLabel       *m_aqiLabel        = nullptr;
    QPushButton  *m_refreshBtn      = nullptr;
    QPushButton  *m_saveBtn         = nullptr;

    // Dane
    std::vector<Station> m_allStations;
    std::vector<Station> m_filteredStations;
};
