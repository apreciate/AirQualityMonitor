#include "MainWindow.h"
#include "StationListWidget.h"
#include "ChartWidget.h"
#include "AnalysisWidget.h"
#include "MapWidget.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QStatusBar>
#include <QProgressBar>
#include <QLabel>
#include <QApplication>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_apiClient(new GiosApiClient(this))
{
    setWindowTitle("Monitor Jakości Powietrza – GIOŚ");
    setMinimumSize(1100, 700);
    setupUi();
    setupMenuBar();
    setupStatusBar();
    connectSignals();
    loadStationsFromApi();
}

// ─── UI Setup ─────────────────────────────────────────────────────────────────

void MainWindow::setupUi()
{
    m_tabs = new QTabWidget(this);

    m_stationList = new StationListWidget(this);
    m_chart       = new ChartWidget(this);
    m_analysis    = new AnalysisWidget(this);
    m_map         = new MapWidget(this);

    m_tabs->addTab(m_stationList, "📋 Stacje");
    m_tabs->addTab(m_chart,       "📈 Wykres");
    m_tabs->addTab(m_analysis,    "🔍 Analiza");
    m_tabs->addTab(m_map,         "🗺️ Mapa");

    setCentralWidget(m_tabs);
}

void MainWindow::setupMenuBar()
{
    QMenu *fileMenu = menuBar()->addMenu("&Plik");
    fileMenu->addAction("&Odśwież stacje", this, &MainWindow::onRefreshStations,
                        QKeySequence::Refresh);
    fileMenu->addAction("&Zapisz dane", this, &MainWindow::onSaveData,
                        QKeySequence::Save);
    fileMenu->addSeparator();
    fileMenu->addAction("&Wyjście", qApp, &QApplication::quit,
                        QKeySequence::Quit);

    QMenu *helpMenu = menuBar()->addMenu("&Pomoc");
    helpMenu->addAction("&O programie", this, [this]() {
        QMessageBox::about(this, "O programie",
            "<h3>Monitor Jakości Powietrza</h3>"
            "<p>Aplikacja pobiera i wizualizuje dane pomiarowe "
            "z sieci GIOŚ (Główny Inspektorat Ochrony Środowiska).</p>"
            "<p><b>Dane:</b> <a href='https://powietrze.gios.gov.pl'>powietrze.gios.gov.pl</a></p>"
            "<p>Projekt zaliczeniowy – Języki Programowania Obiektowego 2025/2026</p>");
    });
}

void MainWindow::setupStatusBar()
{
    m_statusLabel = new QLabel("Gotowy", this);
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);  // nieskończony spinner
    m_progressBar->setFixedWidth(120);
    m_progressBar->setVisible(false);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addPermanentWidget(m_progressBar);
}

void MainWindow::connectSignals()
{
    connect(m_stationList, &StationListWidget::stationSelected,
            this, &MainWindow::onStationSelected);
    connect(m_stationList, &StationListWidget::sensorSelected,
            this, &MainWindow::onSensorSelected);
    connect(m_stationList, &StationListWidget::refreshRequested,
            this, &MainWindow::onRefreshStations);
    connect(m_stationList, &StationListWidget::saveRequested,
            this, &MainWindow::onSaveData);

    connect(m_apiClient, &GiosApiClient::loadingStarted,
            this, &MainWindow::onLoadingStarted);
    connect(m_apiClient, &GiosApiClient::loadingFinished,
            this, &MainWindow::onLoadingFinished);
}

// ─── Stacje ───────────────────────────────────────────────────────────────────

void MainWindow::loadStationsFromApi()
{
    m_statusLabel->setText("Pobieranie stacji z API...");

    m_apiClient->fetchAllStations(
        [this](std::vector<Station> stations) {
            m_stations = std::move(stations);
            m_stationList->setStations(m_stations);
            m_map->setStations(m_stations);
            m_statusLabel->setText(
                QString("Załadowano %1 stacji.").arg(m_stations.size()));
        },
        [this](const QString &err) {
            onApiError(err, [this]() { loadStationsFromDb(); });
        }
    );
}

void MainWindow::loadStationsFromDb()
{
    auto stations = LocalDatabase::instance().loadStations();
    if (stations.empty()) {
        showError("Brak danych lokalnych. Sprawdź połączenie z Internetem.");
        return;
    }
    m_stations = std::move(stations);
    m_stationList->setStations(m_stations);
    m_map->setStations(m_stations);
    m_statusLabel->setText(
        QString("Załadowano %1 stacji z lokalnej bazy.").arg(m_stations.size()));
    showInfo("Brak połączenia z API – wyświetlane dane historyczne z lokalnej bazy.");
}

// ─── Sloty ────────────────────────────────────────────────────────────────────

void MainWindow::onRefreshStations()
{
    m_stationList->clearSensors();
    loadStationsFromApi();
}

void MainWindow::onStationSelected(int stationId)
{
    m_currentStationId = stationId;
    m_statusLabel->setText(QString("Pobieranie sensorów dla stacji %1...").arg(stationId));

    // Indeks jakości powietrza
    m_apiClient->fetchAirQualityIndex(stationId,
        [this](AirQualityIndex aqi) {
            m_stationList->setAirQualityIndex(aqi);
            LocalDatabase::instance().saveAirQualityIndex(aqi);
        },
        [this](const QString &err) {
            // Próba z lokalnej bazy
            auto aqi = LocalDatabase::instance().loadAirQualityIndex(m_currentStationId);
            if (aqi) m_stationList->setAirQualityIndex(*aqi);
            else m_statusLabel->setText("Brak danych indeksu: " + err);
        }
    );

    // Sensory
    m_apiClient->fetchSensors(stationId,
        [this, stationId](std::vector<Sensor> sensors) {
            m_sensors = std::move(sensors);
            m_stationList->setSensors(m_sensors);
            LocalDatabase::instance().saveSensors(stationId, m_sensors);
            m_statusLabel->setText(
                QString("Znaleziono %1 sensorów.").arg(m_sensors.size()));
        },
        [this, stationId](const QString &err) {
            auto sensors = LocalDatabase::instance().loadSensors(stationId);
            if (!sensors.empty()) {
                m_sensors = std::move(sensors);
                m_stationList->setSensors(m_sensors);
                showInfo("Sensory z lokalnej bazy danych.");
            } else {
                showError("Nie można pobrać sensorów: " + err);
            }
        }
    );
}

void MainWindow::onSensorSelected(int sensorId)
{
    m_currentSensorId = sensorId;
    m_statusLabel->setText(QString("Pobieranie pomiarów dla sensora %1...").arg(sensorId));

    m_apiClient->fetchMeasurements(sensorId,
        [this](Measurement m) {
            m_currentMeasurement = std::move(m);
            m_chart->setMeasurement(m_currentMeasurement);
            m_analysis->setMeasurement(m_currentMeasurement);
            LocalDatabase::instance().saveMeasurement(m_currentMeasurement);
            m_tabs->setCurrentWidget(m_chart);
            m_statusLabel->setText(
                QString("Załadowano %1 punktów pomiarowych.")
                .arg(m_currentMeasurement.values.size()));
        },
        [this](const QString &err) {
            auto m = LocalDatabase::instance().loadMeasurement(m_currentSensorId);
            if (m) {
                m_currentMeasurement = *m;
                m_chart->setMeasurement(m_currentMeasurement);
                m_analysis->setMeasurement(m_currentMeasurement);
                m_tabs->setCurrentWidget(m_chart);
                showInfo("Pomiary z lokalnej bazy danych.");
            } else {
                showError("Nie można pobrać pomiarów: " + err);
            }
        }
    );
}

void MainWindow::onSaveData()
{
    if (m_stations.empty()) {
        showError("Brak danych do zapisania.");
        return;
    }
    try {
        LocalDatabase::instance().saveStations(m_stations);
        if (!m_sensors.empty() && m_currentStationId != -1)
            LocalDatabase::instance().saveSensors(m_currentStationId, m_sensors);
        if (!m_currentMeasurement.values.empty())
            LocalDatabase::instance().saveMeasurement(m_currentMeasurement);

        showInfo(QString("Dane zapisano w: %1")
                 .arg(LocalDatabase::instance().dataDirectory()));
    } catch (const std::exception &e) {
        showError(QString("Błąd zapisu: %1").arg(e.what()));
    }
}

void MainWindow::onLoadingStarted()  { m_progressBar->setVisible(true); }
void MainWindow::onLoadingFinished() { m_progressBar->setVisible(false); }

void MainWindow::onApiError(const QString &message, std::function<void()> fallback)
{
    int choice = QMessageBox::warning(this, "Błąd połączenia",
        message + "\n\nCzy chcesz użyć danych z lokalnej bazy?",
        QMessageBox::Yes | QMessageBox::No);
    if (choice == QMessageBox::Yes && fallback)
        fallback();
}

void MainWindow::showError(const QString &msg)
{
    QMessageBox::critical(this, "Błąd", msg);
    m_statusLabel->setText("Błąd: " + msg);
}

void MainWindow::showInfo(const QString &msg)
{
    QMessageBox::information(this, "Informacja", msg);
    m_statusLabel->setText(msg);
}
