#pragma once

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <vector>
#include "../models/Station.h"

/**
 * @brief Widget prezentujący stacje pomiarowe w formie tabeli z filtrowaniem.
 */
class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setStations(const std::vector<Station> &stations);

private slots:
    void onFilterChanged(const QString &text);
    void onProvinceChanged(const QString &province);
    void onOpenInBrowser();
    void onTableSelectionChanged();

private:
    void setupUi();
    void refreshTable();

    QLineEdit    *m_searchEdit    = nullptr;
    QComboBox    *m_provinceCombo = nullptr;
    QTableWidget *m_table         = nullptr;
    QPushButton  *m_browserBtn    = nullptr;
    QLabel       *m_infoLabel     = nullptr;
    QLabel       *m_coordLabel    = nullptr;

    std::vector<Station> m_allStations;
    std::vector<Station> m_filtered;
};