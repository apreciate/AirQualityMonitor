#include "AnalysisWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QFont>
#include <QFrame>

AnalysisWidget::AnalysisWidget(QWidget *parent) : QWidget(parent)
{
    setupUi();
}

void AnalysisWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);

    // ── Zakres analizy ────────────────────────────────────────────────────
    auto *rangeGroup  = new QGroupBox("Zakres analizy", this);
    auto *rangeLayout = new QHBoxLayout(rangeGroup);

    m_fromEdit = new QDateTimeEdit(QDateTime::currentDateTime().addDays(-7), this);
    m_fromEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_toEdit   = new QDateTimeEdit(QDateTime::currentDateTime(), this);
    m_toEdit->setDisplayFormat("yyyy-MM-dd HH:mm");
    m_analyzeBtn = new QPushButton("📊 Analizuj", this);

    rangeLayout->addWidget(new QLabel("Od:"));
    rangeLayout->addWidget(m_fromEdit);
    rangeLayout->addWidget(new QLabel("Do:"));
    rangeLayout->addWidget(m_toEdit);
    rangeLayout->addWidget(m_analyzeBtn);
    rangeLayout->addStretch();
    mainLayout->addWidget(rangeGroup);

    // ── Karty statystyk ───────────────────────────────────────────────────
    auto *statsGroup  = new QGroupBox("Statystyki", this);
    auto *statsLayout = new QGridLayout(statsGroup);
    statsLayout->setSpacing(12);

    auto makeCard = [&](const QString &title, int row, int col,
                        QLabel *&valueLabel, QLabel *&dateLabel) {
        auto *card = new QFrame(statsGroup);
        card->setFrameShape(QFrame::StyledPanel);
        card->setStyleSheet("QFrame { border-radius: 6px; background: #f5f5f5; }");
        auto *cl = new QVBoxLayout(card);
        auto *titleL = new QLabel(title, card);
        QFont tf; tf.setBold(true); tf.setPointSize(9);
        titleL->setFont(tf);
        titleL->setAlignment(Qt::AlignCenter);
        valueLabel = new QLabel("–", card);
        QFont vf; vf.setPointSize(16); vf.setBold(true);
        valueLabel->setFont(vf);
        valueLabel->setAlignment(Qt::AlignCenter);
        dateLabel = new QLabel("", card);
        dateLabel->setAlignment(Qt::AlignCenter);
        dateLabel->setStyleSheet("color: #666;");
        cl->addWidget(titleL);
        cl->addWidget(valueLabel);
        cl->addWidget(dateLabel);
        statsLayout->addWidget(card, row, col);
    };

    makeCard("🔽 Minimum",  0, 0, m_minLabel, m_minDate);
    makeCard("🔼 Maksimum", 0, 1, m_maxLabel, m_maxDate);
    makeCard("📊 Średnia",  0, 2, m_avgLabel, m_countLabel);

    m_trendLabel = new QLabel("Trend: –", statsGroup);
    QFont tf; tf.setPointSize(13); tf.setBold(true);
    m_trendLabel->setFont(tf);
    m_trendLabel->setAlignment(Qt::AlignCenter);
    m_trendLabel->setFrameShape(QFrame::StyledPanel);
    statsLayout->addWidget(m_trendLabel, 1, 0, 1, 3);

    mainLayout->addWidget(statsGroup);

    // ── Tabela danych ─────────────────────────────────────────────────────
    auto *tableGroup  = new QGroupBox("Szczegółowe dane pomiarowe", this);
    auto *tableLayout = new QVBoxLayout(tableGroup);
    m_table = new QTableWidget(0, 2, tableGroup);
    m_table->setHorizontalHeaderLabels({"Data i czas", "Wartość"});
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_table->setAlternatingRowColors(true);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setSortingEnabled(true);
    tableLayout->addWidget(m_table);
    mainLayout->addWidget(tableGroup, 1);

    connect(m_analyzeBtn, &QPushButton::clicked, this, &AnalysisWidget::onAnalyzeRange);
}

void AnalysisWidget::setMeasurement(const Measurement &measurement)
{
    m_measurement = measurement;
    // Ustaw zakres na pełny dostępny
    if (!measurement.values.empty()) {
        m_fromEdit->setDateTime(measurement.values.back().date);
        m_toEdit->setDateTime(measurement.values.front().date);
    }
    updateAnalysis(measurement);
}

void AnalysisWidget::onAnalyzeRange()
{
    Measurement filtered = m_measurement.filtered(m_fromEdit->dateTime(),
                                                   m_toEdit->dateTime());
    updateAnalysis(filtered);
}

void AnalysisWidget::updateAnalysis(const Measurement &m)
{
    // Wypełnij tabelę
    m_table->setRowCount(0);
    for (const auto &p : m.values) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(
            p.date.toString("yyyy-MM-dd HH:mm:ss")));
        m_table->setItem(row, 1, new QTableWidgetItem(
            p.value.has_value() ? QString::number(p.value.value(), 'f', 2) : "null"));
    }

    // Analiza statystyczna
    auto result = DataAnalyzer::analyze(m);
    if (!result) {
        m_minLabel->setText("–");
        m_maxLabel->setText("–");
        m_avgLabel->setText("–");
        m_trendLabel->setText("Trend: brak danych");
        return;
    }

    m_minLabel->setText(QString::number(result->minValue, 'f', 2));
    m_minDate->setText(result->minDate.toString("dd.MM.yyyy HH:mm"));

    m_maxLabel->setText(QString::number(result->maxValue, 'f', 2));
    m_maxDate->setText(result->maxDate.toString("dd.MM.yyyy HH:mm"));

    m_avgLabel->setText(QString::number(result->avgValue, 'f', 2));
    m_countLabel->setText(
        QString("%1 / %2 pomiarów").arg(result->validCount).arg(result->totalCount));

    m_trendLabel->setText(
        QString("Trend: %1  (współczynnik: %2)")
        .arg(result->trendDescription())
        .arg(result->trend, 0, 'f', 4));

    // Kolor trendu
    QString trendColor;
    if (result->trend > 0.01)       trendColor = "#ffcccc";
    else if (result->trend < -0.01) trendColor = "#ccffcc";
    else                            trendColor = "#ffffcc";
    m_trendLabel->setStyleSheet(
        QString("background: %1; border-radius: 4px; padding: 6px;").arg(trendColor));
}
