#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include <QSplitter>
#include "../models/Measurement.h"
#include "../database/LocalDatabase.h"

/**
 * @brief Widget historii zapisanych pomiarów.
 */
class HistoryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryWidget(QWidget *parent = nullptr);
    void refresh();

signals:
    void measurementSelected(const Measurement &m);

private slots:
    void onItemClicked(QListWidgetItem *item);
    void onDeleteClicked();

private:
    void setupUi();

    QListWidget *m_list       = nullptr;
    QLabel      *m_infoLabel  = nullptr;
    QPushButton *m_deleteBtn  = nullptr;
    QPushButton *m_refreshBtn = nullptr;
};