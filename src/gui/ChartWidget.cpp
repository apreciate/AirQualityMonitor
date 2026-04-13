#include "ChartWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QPixmap>
#include <QDateTime>

#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <QtCharts/QScatterSeries>

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void ChartWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(6);

    // ── Zakres czasu ──────────────────────────────────────────────────────
    auto *rangeGroup  = new QGroupBox("Zakres danych", this);
    auto *rangeLayout = new QHBoxLayout(rangeGroup);

    m_presetCombo = new QComboBox(this);
    m_presetCombo->addItems({"Ostatnie 24h", "Ostatnie 3 dni",
                              "Ostatni tydzień", "Ostatnie 2 tygodnie",
                              "Wszystkie dane", "Własny zakres"});

    rangeLayout->addWidget(new QLabel("Preset:", this));
    rangeLayout->addWidget(m_presetCombo);
    rangeLayout->addSpacing(16);

    m_fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-1), this);
    m_fromEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_fromEdit->setEnabled(false);
    m_toEdit = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_toEdit->setEnabled(false);

    m_applyBtn = new QPushButton("Zastosuj", this);
    m_applyBtn->setEnabled(false);

    rangeLayout->addWidget(new QLabel("Od:", this));
    rangeLayout->addWidget(m_fromEdit);
    rangeLayout->addWidget(new QLabel("Do:", this));
    rangeLayout->addWidget(m_toEdit);
    rangeLayout->addWidget(m_applyBtn);
    rangeLayout->addStretch();

    m_exportBtn = new QPushButton("📷 Eksportuj PNG", this);
    rangeLayout->addWidget(m_exportBtn);

    mainLayout->addWidget(rangeGroup);

    // ── Wykres ────────────────────────────────────────────────────────────
    auto *chart = new QChart();
    chart->setTitle("Brak danych – wybierz stację i sensor");
    chart->setAnimationOptions(QChart::SeriesAnimations);
    chart->legend()->setVisible(true);

    m_chartView = new QChartView(chart, this);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    mainLayout->addWidget(m_chartView, 1);

    // ── Informacje ────────────────────────────────────────────────────────
    m_infoLabel = new QLabel("", this);
    m_infoLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_infoLabel);

    // ── Połączenia ────────────────────────────────────────────────────────
    connect(m_presetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ChartWidget::onRangePresetChanged);
    connect(m_applyBtn,  &QPushButton::clicked, this, &ChartWidget::onCustomRangeApplied);
    connect(m_exportBtn, &QPushButton::clicked, this, &ChartWidget::onExportChart);
}

// ─── Public ───────────────────────────────────────────────────────────────────

void ChartWidget::setMeasurement(const Measurement &measurement)
{
    m_measurement = measurement;
    m_presetCombo->setCurrentIndex(0);
    onRangePresetChanged(0);
}

// ─── Sloty ────────────────────────────────────────────────────────────────────

void ChartWidget::onRangePresetChanged(int index)
{
    bool custom = (index == 5);
    m_fromEdit->setEnabled(custom);
    m_toEdit->setEnabled(custom);
    m_applyBtn->setEnabled(custom);

    if (custom) return;

    QDateTime to   = QDateTime::currentDateTime();
    QDateTime from = to;

    switch (index) {
    case 0: from = to.addSecs(-24*3600); break;  // 24h
    case 1: from = to.addDays(-3);       break;  // 3 dni
    case 2: from = to.addDays(-7);       break;  // tydzień
    case 3: from = to.addDays(-14);      break;  // 2 tygodnie
    case 4: from = QDateTime(QDate(2000,1,1), QTime(0,0)); break;
    default: break;
    }

    m_fromEdit->setDateTime(from);
    m_toEdit->setDateTime(to);
    applyRange(from, to);
}

void ChartWidget::onCustomRangeApplied()
{
    applyRange(m_fromEdit->dateTime(), m_toEdit->dateTime());
}

void ChartWidget::applyRange(const QDateTime &from, const QDateTime &to)
{
    Measurement filtered = m_measurement.filtered(from, to);
    m_measurement = m_measurement;  // keep original
    // Rebuild chart with filtered data
    auto *chart = m_chartView->chart();
    chart->removeAllSeries();
    for (auto *axis : chart->axes())
        chart->removeAxis(axis);

    auto *series = new QLineSeries();
    series->setName(m_measurement.key);
    QPen pen(QColor("#2196F3"));
    pen.setWidth(2);
    series->setPen(pen);

    int validPts = 0;
    for (const auto &p : filtered.values) {
        if (p.value.has_value()) {
            series->append(p.date.toMSecsSinceEpoch(), p.value.value());
            ++validPts;
        }
    }

    chart->addSeries(series);
    chart->setTitle(QString("Parametr: %1").arg(m_measurement.key));

    auto *axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM HH:mm");
    axisX->setTitleText("Data i czas");
    axisX->setTickCount(8);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);

    auto *axisY = new QValueAxis();
    axisY->setTitleText("Wartość");
    axisY->setLabelFormat("%.2f");
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    m_infoLabel->setText(
        QString("Wyświetlono %1 punktów pomiarowych | Zakres: %2 – %3")
        .arg(validPts)
        .arg(from.toString("yyyy-MM-dd HH:mm"))
        .arg(to.toString("yyyy-MM-dd HH:mm")));
}

void ChartWidget::onExportChart()
{
    QString path = QFileDialog::getSaveFileName(
        this, "Zapisz wykres", "wykres.png", "Obrazy PNG (*.png)");
    if (path.isEmpty()) return;

    QPixmap pix = m_chartView->grab();
    if (pix.save(path))
        m_infoLabel->setText("Wykres zapisano: " + path);
    else
        m_infoLabel->setText("Błąd zapisu wykresu.");
}
