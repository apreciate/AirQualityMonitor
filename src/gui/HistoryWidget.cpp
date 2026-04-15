#include "HistoryWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QMessageBox>

HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent) { setupUi(); }

void HistoryWidget::setupUi()
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 8, 8, 8);

    auto *infoBar = new QLabel(
        "📂 Historia zapisanych pomiarów. "
        "Kliknij wpis aby załadować dane na wykres.", this);
    infoBar->setStyleSheet(
        "background:#f3e5f5;border:1px solid #ce93d8;"
        "border-radius:4px;padding:8px;");
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
        // Wyciągnij sensorId z nazwy pliku
        QString idStr = file;
        idStr.remove("measurements_").remove(".json");
        int sensorId = idStr.toInt();

        auto m = LocalDatabase::instance().loadMeasurement(sensorId);
        if (!m) continue;

        QFileInfo fi(dir + "/" + file);
        QString dateStr = fi.lastModified().toString("dd.MM.yyyy HH:mm");

        QString label = QString("Sensor %1 | %2 | %3 pkt | zapisano: %4")
                            .arg(sensorId)
                            .arg(m->key.isEmpty() ? "–" : m->key)
                            .arg(m->values.size())
                            .arg(dateStr);

        auto *item = new QListWidgetItem(label, m_list);
        item->setData(Qt::UserRole, sensorId);
    }

    m_infoLabel->setText(QString("Zapisanych pomiarów: %1").arg(files.size()));
}

void HistoryWidget::onItemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int sensorId = item->data(Qt::UserRole).toInt();
    auto m = LocalDatabase::instance().loadMeasurement(sensorId);
    if (m) emit measurementSelected(*m);
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