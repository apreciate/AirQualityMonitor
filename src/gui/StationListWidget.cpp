#include "StationListWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QGroupBox>
#include <QListWidgetItem>
#include <algorithm>

StationListWidget::StationListWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void StationListWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ── Filtry ─────────────────────────────────────────────────────────────
    auto *filterGroup = new QGroupBox("Filtruj stacje", this);
    auto *filterLayout = new QHBoxLayout(filterGroup);

    m_filterEdit = new QLineEdit(this);
    m_filterEdit->setPlaceholderText("Wpisz nazwę miejscowości...");
    m_filterEdit->setClearButtonEnabled(true);

    m_provinceCombo = new QComboBox(this);
    m_provinceCombo->addItem("Wszystkie województwa");
    m_provinceCombo->setMinimumWidth(180);

    filterLayout->addWidget(new QLabel("Miasto:", this));
    filterLayout->addWidget(m_filterEdit, 1);
    filterLayout->addWidget(new QLabel("Województwo:", this));
    filterLayout->addWidget(m_provinceCombo);

    mainLayout->addWidget(filterGroup);

    // ── Splitter: stacje | sensory ─────────────────────────────────────────
    auto *splitter = new QSplitter(Qt::Horizontal, this);

    auto *stationBox = new QGroupBox("Stacje pomiarowe", splitter);
    auto *stationLayout = new QVBoxLayout(stationBox);
    m_stationList = new QListWidget(stationBox);
    m_stationList->setAlternatingRowColors(true);
    stationLayout->addWidget(m_stationList);

    auto *sensorBox = new QGroupBox("Stanowiska pomiarowe (sensory)", splitter);
    auto *sensorLayout = new QVBoxLayout(sensorBox);
    m_sensorList = new QListWidget(sensorBox);
    m_sensorList->setAlternatingRowColors(true);
    sensorLayout->addWidget(m_sensorList);

    splitter->addWidget(stationBox);
    splitter->addWidget(sensorBox);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);

    mainLayout->addWidget(splitter, 1);

    // ── Indeks jakości powietrza ───────────────────────────────────────────
    m_aqiLabel = new QLabel("Indeks jakości powietrza: (wybierz stację)", this);
    m_aqiLabel->setAlignment(Qt::AlignCenter);
    m_aqiLabel->setFrameShape(QFrame::StyledPanel);
    m_aqiLabel->setFixedHeight(36);
    QFont f = m_aqiLabel->font();
    f.setBold(true);
    m_aqiLabel->setFont(f);
    mainLayout->addWidget(m_aqiLabel);

    // ── Przyciski ─────────────────────────────────────────────────────────
    auto *btnLayout = new QHBoxLayout;
    m_refreshBtn = new QPushButton("🔄 Odśwież stacje", this);
    m_saveBtn    = new QPushButton("💾 Zapisz dane", this);
    btnLayout->addStretch();
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addWidget(m_saveBtn);
    mainLayout->addLayout(btnLayout);

    // ── Połączenia ─────────────────────────────────────────────────────────
    connect(m_stationList, &QListWidget::itemClicked,
            this, &StationListWidget::onStationClicked);
    connect(m_sensorList,  &QListWidget::itemClicked,
            this, &StationListWidget::onSensorClicked);
    connect(m_filterEdit, &QLineEdit::textChanged,
            this, &StationListWidget::onFilterChanged);
    connect(m_provinceCombo, &QComboBox::currentTextChanged,
            this, &StationListWidget::onProvinceChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &StationListWidget::refreshRequested);
    connect(m_saveBtn,    &QPushButton::clicked, this, &StationListWidget::saveRequested);
}

// ─── Public setters ───────────────────────────────────────────────────────────

void StationListWidget::setStations(const std::vector<Station> &stations)
{
    m_allStations = stations;

    // Wypełnij kombo województw
    m_provinceCombo->blockSignals(true);
    m_provinceCombo->clear();
    m_provinceCombo->addItem("Wszystkie województwa");
    QStringList provinces;
    for (const auto &s : stations)
        if (!s.province.isEmpty() && !provinces.contains(s.province))
            provinces.append(s.province);
    provinces.sort();
    for (const auto &p : provinces)
        m_provinceCombo->addItem(p);
    m_provinceCombo->blockSignals(false);

    m_filteredStations = stations;
    refreshStationList();
}

void StationListWidget::setSensors(const std::vector<Sensor> &sensors)
{
    m_sensorList->clear();
    for (const auto &s : sensors) {
        auto *item = new QListWidgetItem(s.displayName(), m_sensorList);
        item->setData(Qt::UserRole, s.id);
    }
}

void StationListWidget::setAirQualityIndex(const AirQualityIndex &aqi)
{
    QString color  = aqi.stationIndex.color();
    QString name   = aqi.stationIndex.name;
    m_aqiLabel->setText(QString("Indeks jakości powietrza: %1").arg(name));
    m_aqiLabel->setStyleSheet(
        QString("background-color: %1; color: %2; border-radius: 4px; padding: 4px;")
        .arg(color)
        .arg(aqi.stationIndex.id <= 1 ? "#000000" : "#ffffff"));
}

void StationListWidget::clearSensors()
{
    m_sensorList->clear();
}

// ─── Sloty ────────────────────────────────────────────────────────────────────

void StationListWidget::onStationClicked(QListWidgetItem *item)
{
    if (!item) return;
    m_sensorList->clear();
    emit stationSelected(item->data(Qt::UserRole).toInt());
}

void StationListWidget::onSensorClicked(QListWidgetItem *item)
{
    if (!item) return;
    emit sensorSelected(item->data(Qt::UserRole).toInt());
}

void StationListWidget::onFilterChanged(const QString &text)
{
    QString province = m_provinceCombo->currentText();
    m_filteredStations.clear();
    for (const auto &s : m_allStations) {
        bool cityMatch     = text.isEmpty() ||
                             s.city.contains(text, Qt::CaseInsensitive) ||
                             s.name.contains(text, Qt::CaseInsensitive);
        bool provinceMatch = (province == "Wszystkie województwa") ||
                             s.province == province;
        if (cityMatch && provinceMatch)
            m_filteredStations.push_back(s);
    }
    refreshStationList();
}

void StationListWidget::onProvinceChanged(const QString &province)
{
    onFilterChanged(m_filterEdit->text());
}

void StationListWidget::refreshStationList()
{
    m_stationList->clear();
    for (const auto &s : m_filteredStations) {
        auto *item = new QListWidgetItem(s.displayName(), m_stationList);
        item->setData(Qt::UserRole, s.id);
        item->setToolTip(
            QString("Adres: %1\nGmina: %2\nPowiat: %3\nWojewództwo: %4\nWsp.: %5, %6")
            .arg(s.street).arg(s.commune).arg(s.district).arg(s.province)
            .arg(s.lat).arg(s.lon));
    }
}
