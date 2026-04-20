#include "ChartWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QPixmap>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent) { setupUi(); }

void ChartWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Pasek trybu (live / historia)
    m_modeLabel = new QLabel("", this);
    m_modeLabel->setAlignment(Qt::AlignCenter);
    m_modeLabel->setVisible(false);
    mainLayout->addWidget(m_modeLabel);

    auto *rangeGroup  = new QGroupBox("Zakres danych", this);
    auto *rangeLayout = new QHBoxLayout(rangeGroup);

    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItems({"Ostatnie 24h", "Ostatnie 3 dni", "Ostatni tydzień",
                              "Ostatnie 2 tygodnie", "Wszystkie dane", "Własny zakres"});

    m_fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1), this);
    m_fromEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_fromEdit->setEnabled(false);

    m_toEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_toEdit->setEnabled(false);

    m_applyBtn  = new QPushButton("Zastosuj", this);
    m_applyBtn->setEnabled(false);
    m_exportBtn = new QPushButton("📷 Eksportuj PNG", this);

    rangeLayout->addWidget(new QLabel("Preset:"));
    rangeLayout->addWidget(m_presetCombo);
    rangeLayout->addWidget(new QLabel("Od:"));
    rangeLayout->addWidget(m_fromEdit);
    rangeLayout->addWidget(new QLabel("Do:"));
    rangeLayout->addWidget(m_toEdit);
    rangeLayout->addWidget(m_applyBtn);
    rangeLayout->addStretch();
    rangeLayout->addWidget(m_exportBtn);
    mainLayout->addWidget(rangeGroup);

    auto *chart = new QChart();
    chart->setTitle("Brak danych – wybierz stację i sensor");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView, 1);

    m_infoLabel = new QLabel("", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_infoLabel);

    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChartWidget::onRangePresetChanged);
    connect(m_applyBtn,  &QPushButton::clicked, this, &ChartWidget::onCustomRangeApplied);
    connect(m_exportBtn, &QPushButton::clicked, this, &ChartWidget::onExportChart);
}

// ─── Tryb live ────────────────────────────────────────────────────────────────

void ChartWidget::setMeasurement(const Measurement &measurement)
{
    m_historyMode = false;
    m_measurement = measurement;
    setControlsLocked(false);
    m_modeLabel->setVisible(false);
    m_presetCombo->setCurrentIndex(0);
    onRangePresetChanged(0);
}

// ─── Tryb historyczny ─────────────────────────────────────────────────────────

void ChartWidget::setHistoricalMeasurement(const Measurement &measurement)
{
    m_historyMode = true;
    m_measurement = measurement;
    setControlsLocked(true);

    // Pokaż baner informujący o trybie historycznym
    m_modeLabel->setText(
        "📂  Tryb historyczny – wyświetlane są zapisane dane. "
        "Wybierz sensor ze stacji aby wrócić do trybu live.");
    m_modeLabel->setStyleSheet(
        "background:#fff3e0; border:1px solid #ffb74d; "
        "border-radius:4px; padding:6px; color:#e65100;");
    m_modeLabel->setVisible(true);

    // Wyświetl wszystkie zapisane punkty bez filtrowania
    if (!measurement.values.empty()) {
        QDateTime earliest = measurement.values.back().date;
        QDateTime latest   = measurement.values.front().date;
        // Upewnij się że kolejność jest właściwa
        for (const auto &p : measurement.values) {
            if (p.date.isValid()) {
                if (p.date < earliest) earliest = p.date;
                if (p.date > latest)   latest   = p.date;
            }
        }
        m_fromEdit->setDateTime(earliest);
        m_toEdit->setDateTime(latest);
        applyRange(earliest, latest);
    }
}

void ChartWidget::setControlsLocked(bool locked)
{
    m_presetCombo->setEnabled(!locked);
    m_fromEdit->setEnabled(false);   // własny zakres tylko przez preset
    m_toEdit->setEnabled(false);
    m_applyBtn->setEnabled(false);
}

// ─── Presety ──────────────────────────────────────────────────────────────────

void ChartWidget::onRangePresetChanged(int index)
{
    // W trybie historycznym presety są zablokowane
    if (m_historyMode) return;

    bool custom = (index == 5);
    m_fromEdit->setEnabled(custom);
    m_toEdit->setEnabled(custom);
    m_applyBtn->setEnabled(custom);
    if (custom) return;

    QDateTime to   = QDateTime::currentDateTime();
    QDateTime from = to;
    switch (index) {
        case 0: from = to.addSecs(-24 * 3600); break;  // 24h
        case 1: from = to.addDays(-3);          break;  // 3 dni
        case 2: from = to.addDays(-7);          break;  // tydzień
        case 3: from = to.addDays(-14);         break;  // 2 tygodnie
        case 4: from = QDateTime(QDate(2000,1,1), QTime(0,0)); break;
        default: break;
    }
    m_fromEdit->setDateTime(from);
    m_toEdit->setDateTime(to);
    applyRange(from, to);
}

void ChartWidget::onCustomRangeApplied()
{
    if (m_historyMode) return;
    applyRange(m_fromEdit->dateTime(), m_toEdit->dateTime());
}

// ─── Rysowanie wykresu z próbkowaniem ─────────────────────────────────────────

void ChartWidget::applyRange(const QDateTime &from, const QDateTime &to)
{
    Measurement filtered = m_measurement.filtered(from, to);

    // Próbkowanie: im szerszy zakres, tym co N-ty punkt jest brany
    // 24h  (~24 pkt)  → krok 1 (wszystkie)
    // 3dni (~72 pkt)  → krok 3
    // 7dni (~168 pkt) → krok 7
    // 14dni(~336 pkt) → krok 14
    qint64 rangeSecs = from.secsTo(to);
    int step = 1;
    if      (rangeSecs > 14 * 86400) step = 14;
    else if (rangeSecs >  7 * 86400) step = 7;
    else if (rangeSecs >  3 * 86400) step = 3;
    else                              step = 1;

    auto *chart = m_chartView->chart();
    chart->removeAllSeries();
    for (auto *axis : chart->axes()) chart->removeAxis(axis);

    auto *series = new QLineSeries();
    series->setName(m_measurement.key);
    QPen pen(QColor("#2196F3")); pen.setWidth(2);
    series->setPen(pen);

    int validPts = 0;
    int i = 0;
    for (const auto &p : filtered.values) {
        if (p.value.has_value() && p.date.isValid()) {
            if (i % step == 0)
                series->append(p.date.toMSecsSinceEpoch(), p.value.value());
            ++validPts;
        }
        ++i;
    }

    chart->addSeries(series);
    chart->setTitle(QString("%1Parametr: %2")
        .arg(m_historyMode ? "📂 HISTORIA | " : "")
        .arg(m_measurement.key.isEmpty() ? "–" : m_measurement.key));

    auto *axisX = new QDateTimeAxis();
    // Format osi X zależy od zakresu
    axisX->setFormat(rangeSecs > 3 * 86400 ? "dd.MM" : "dd.MM HH:mm");
    axisX->setTitleText("Data i czas");
    axisX->setTickCount(rangeSecs > 7 * 86400 ? 15 : 8);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QValueAxis();
    axisY->setTitleText("Wartość [µg/m³]");
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_infoLabel->setText(
        QString("%1Wyświetlono %2 z %3 punktów | krok próbkowania: %4 | Zakres: %5 – %6")
        .arg(m_historyMode ? "📂 " : "")
        .arg(series->count())
        .arg(validPts)
        .arg(step)
        .arg(from.toString("yyyy-MM-dd HH:mm"))
        .arg(to.toString("yyyy-MM-dd HH:mm")));
}

void ChartWidget::onExportChart()
{
    QString path = QFileDialog::getSaveFileName(this, "Zapisz wykres", "wykres.png", "PNG (*.png)");
    if (path.isEmpty()) return;
    QPixmap pix = m_chartView->grab();
    m_infoLabel->setText(pix.save(path) ? "Wykres zapisano: " + path : "Błąd zapisu.");
}
