#include "MapWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QDesktopServices>
#include <QUrl>
#include <QGroupBox>
#include <QTableWidgetItem>

MapWidget::MapWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void MapWidget::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);
    layout->setSpacing(6);

    auto *banner = new QLabel(
        "🗺️  <b>Widok mapy</b> — Qt WebEngine niedostępny dla MinGW. "
        "Stacje wyświetlane w tabeli. Wybierz stację i otwórz w przeglądarce.", this);
    banner->setWordWrap(true);
    banner->setStyleSheet(
        "background:#e3f2fd; border:1px solid #90caf9; "
        "border-radius:4px; padding:8px; color:#0d47a1;");
    layout->addWidget(banner);

    auto *filterGroup  = new QGroupBox("Filtruj stacje", this);
    auto *filterLayout = new QHBoxLayout(filterGroup);
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Szukaj po nazwie lub mieście...");
    m_searchEdit->setClearButtonEnabled(true);
    m_provinceCombo = new QComboBox(this);
    m_provinceCombo->addItem("Wszystkie województwa");
    filterLayout->addWidget(new QLabel("Szukaj:"));
    filterLayout->addWidget(m_searchEdit, 1);
    filterLayout->addWidget(new QLabel("Województwo:"));
    filterLayout->addWidget(m_provinceCombo);
    layout->addWidget(filterGroup);

    m_table = new QTableWidget(0, 6, this);
    m_table->setHorizontalHeaderLabels({
        "ID", "Nazwa stacji", "Miasto", "Województwo", "Szer. geogr.", "Dług. geogr."
    });
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSortingEnabled(true);
    layout->addWidget(m_table, 1);

    auto *bottomLayout = new QHBoxLayout;
    m_coordLabel = new QLabel("Wybierz stację, aby zobaczyć współrzędne.", this);
    m_coordLabel->setStyleSheet("color: #555;");
    m_browserBtn = new QPushButton("🌐 Otwórz w Google Maps", this);
    m_browserBtn->setEnabled(false);
    m_infoLabel = new QLabel("", this);
    bottomLayout->addWidget(m_coordLabel, 1);
    bottomLayout->addWidget(m_browserBtn);
    layout->addLayout(bottomLayout);
    layout->addWidget(m_infoLabel);

    connect(m_searchEdit,    &QLineEdit::textChanged,
            this, &MapWidget::onFilterChanged);
    connect(m_provinceCombo, &QComboBox::currentTextChanged,
            this, &MapWidget::onProvinceChanged);
    connect(m_browserBtn,    &QPushButton::clicked,
            this, &MapWidget::onOpenInBrowser);
    connect(m_table, &QTableWidget::itemSelectionChanged,
            this, &MapWidget::onTableSelectionChanged);
}

void MapWidget::setStations(const std::vector<Station> &stations)
{
    m_allStations = stations;
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
    m_filtered = stations;
    refreshTable();
    m_infoLabel->setText(QString("Załadowano %1 stacji.").arg(stations.size()));
}

void MapWidget::onFilterChanged(const QString &text)
{
    QString province = m_provinceCombo->currentText();
    m_filtered.clear();
    for (const auto &s : m_allStations) {
        bool textMatch = text.isEmpty()
        || s.name.contains(text, Qt::CaseInsensitive)
            || s.city.contains(text, Qt::CaseInsensitive);
        bool provMatch = (province == "Wszystkie województwa")
                         || s.province == province;
        if (textMatch && provMatch)
            m_filtered.push_back(s);
    }
    refreshTable();
}

void MapWidget::onProvinceChanged(const QString &)
{
    onFilterChanged(m_searchEdit->text());
}

void MapWidget::onTableSelectionChanged()
{
    if (m_table->selectedItems().isEmpty()) {
        m_coordLabel->setText("Wybierz stację, aby zobaczyć współrzędne.");
        m_browserBtn->setEnabled(false);
        return;
    }
    int row = m_table->currentRow();
    QString lat  = m_table->item(row, 4)->text();
    QString lon  = m_table->item(row, 5)->text();
    QString name = m_table->item(row, 1)->text();
    m_coordLabel->setText(
        QString("📍 %1  |  Lat: %2  |  Lon: %3").arg(name).arg(lat).arg(lon));
    m_browserBtn->setEnabled(true);
}

void MapWidget::onOpenInBrowser()
{
    int row = m_table->currentRow();
    if (row < 0) return;

    // Pobierz wartości numeryczne bezpośrednio z modelu (nie z tekstu komórki)
    double lat = m_table->item(row, 4)->data(Qt::DisplayRole).toDouble();
    double lon = m_table->item(row, 5)->data(Qt::DisplayRole).toDouble();

    // Użyj 'f' z locale C żeby zawsze była kropka, nie przecinek
    QString latStr = QString::number(lat, 'f', 6);
    QString lonStr = QString::number(lon, 'f', 6);

    QDesktopServices::openUrl(QUrl(
        QString("https://www.google.com/maps?q=%1,%2").arg(latStr, lonStr)));
}

void MapWidget::refreshTable()
{
    m_table->setSortingEnabled(false);
    m_table->setRowCount(0);
    for (const auto &s : m_filtered) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        auto *idItem = new QTableWidgetItem;
        idItem->setData(Qt::DisplayRole, s.id);
        m_table->setItem(row, 0, idItem);
        m_table->setItem(row, 1, new QTableWidgetItem(s.name));
        m_table->setItem(row, 2, new QTableWidgetItem(s.city));
        m_table->setItem(row, 3, new QTableWidgetItem(s.province));
        auto *latItem = new QTableWidgetItem;
        latItem->setData(Qt::DisplayRole, s.lat);
        auto *lonItem = new QTableWidgetItem;
        lonItem->setData(Qt::DisplayRole, s.lon);
        m_table->setItem(row, 4, latItem);
        m_table->setItem(row, 5, lonItem);
    }
    m_table->setSortingEnabled(true);
    m_infoLabel->setText(QString("Wyświetlono %1 stacji.").arg(m_filtered.size()));
}