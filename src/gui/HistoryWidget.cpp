#include "HistoryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>

HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent) { setupUi(); }

void HistoryWidget::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto *infoBar = new QLabel(
        "📂 Historia zapisanych pomiarów. "
        "Kliknij wpis aby załadować dane na wykres w trybie historycznym.", this);
    infoBar->setWordWrap(true);
    infoBar->setStyleSheet(
        "background:#f3e5f5; border:1px solid #ce93d8; "
        "border-radius:4px; padding:8px;");
    layout->addWidget(infoBar);

    m_list = new QListWidget(this);
    m_list->setAlternatingRowColors(true);
    layout->addWidget(m_list, 1);

    auto *btnLayout = new QHBoxLayout;
    m_infoLabel  = new QLabel("", this);
    m_refreshBtn = new QPushButton("🔄 Odśwież", this);
    m_deleteBtn  = new QPushButton("🗑️ Usuń zaznaczone", this);
    m_deleteBtn->setEnabled(false);
    btnLayout->addWidget(m_infoLabel, 1);
    btnLayout->addWidget(m_refreshBtn);
    btnLayout->addWidget(m_deleteBtn);
    layout->addLayout(btnLayout);

    connect(m_list, &QListWidget::itemClicked,
            this, &HistoryWidget::onItemClicked);
    connect(m_refreshBtn, &QPushButton::clicked,
            this, &HistoryWidget::refresh);
    connect(m_deleteBtn, &QPushButton::clicked,
            this, &HistoryWidget::onDeleteClicked);
    connect(m_list, &QListWidget::itemSelectionChanged, this, [this]() {
        m_deleteBtn->setEnabled(!m_list->selectedItems().isEmpty());
    });
}

void HistoryWidget::refresh()
{
    m_list->clear();
    QString dir = LocalDatabase::instance().dataDirectory();
    QDir d(dir);
    QStringList files = d.entryList({"measurements_*.json"}, QDir::Files, QDir::Time);

    for (const QString &file : files) {
        QString idStr = file;
        idStr.remove("measurements_").remove(".json");
        int sensorId = idStr.toInt();

        auto m = LocalDatabase::instance().loadMeasurement(sensorId);
        if (!m) continue;

        QFileInfo fi(dir + "/" + file);
        QString savedAt = fi.lastModified().toString("dd.MM.yyyy HH:mm");

        // Oblicz rzeczywisty zakres dat zapisanych pomiarów
        QDateTime earliest, latest;
        for (const auto &p : m->values) {
            if (!p.date.isValid()) continue;
            if (!earliest.isValid() || p.date < earliest) earliest = p.date;
            if (!latest.isValid()   || p.date > latest)   latest   = p.date;
        }

        QString rangeStr = (earliest.isValid() && latest.isValid())
            ? QString("%1 → %2")
              .arg(earliest.toString("dd.MM HH:mm"))
              .arg(latest.toString("dd.MM HH:mm"))
            : "brak dat";

        QString label = QString("Sensor %1 | %2 | %3 pkt | %4 | zapisano: %5")
            .arg(sensorId)
            .arg(m->key.isEmpty() ? "–" : m->key)
            .arg(m->values.size())
            .arg(rangeStr)
            .arg(savedAt);

        auto *item = new QListWidgetItem(label, m_list);
        item->setData(Qt::UserRole, sensorId);
        item->setToolTip(
            QString("Kliknij aby załadować %1 punktów pomiarowych\n"
                    "Zakres: %2\nOstatni zapis: %3")
            .arg(m->values.size()).arg(rangeStr).arg(savedAt));
    }

    m_infoLabel->setText(QString("Zapisanych: %1 sensorów").arg(files.size()));
}

void HistoryWidget::onItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int sensorId = item->data(Qt::UserRole).toInt();
    auto m = LocalDatabase::instance().loadMeasurement(sensorId);
    if (m)
        emit measurementSelected(*m);
}

void HistoryWidget::onDeleteClicked()
{
    auto selected = m_list->selectedItems();
    if (selected.isEmpty()) return;

    int choice = QMessageBox::question(this, "Usuń",
        QString("Usunąć %1 zapisanych pomiarów?").arg(selected.size()),
        QMessageBox::Yes | QMessageBox::No);
    if (choice != QMessageBox::Yes) return;

    QString dir = LocalDatabase::instance().dataDirectory();
    for (auto *item : selected) {
        int sensorId = item->data(Qt::UserRole).toInt();
        QFile::remove(dir + QString("/measurements_%1.json").arg(sensorId));
        delete item;
    }
    refresh();
}
