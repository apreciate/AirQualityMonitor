#pragma once

#include <QWidget>
#include <QListWidget>
#include <QLabel>
#include <QPushButton>
#include "../models/Measurement.h"
#include "../database/LocalDatabase.h"

/**
 * @brief Widget historii zapisanych pomiarów.
 *
 * Wyświetla listę plików pomiarowych zapisanych w lokalnej bazie danych.
 * Po kliknięciu wpisu emituje sygnał @ref measurementSelected,
 * który ładuje dane na wykres w trybie historycznym.
 *
 * Umożliwia też usuwanie wybranych wpisów z bazy.
 */
class HistoryWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor.
     * @param parent Opcjonalny rodzic Qt.
     */
    explicit HistoryWidget(QWidget *parent = nullptr);

    /**
     * @brief Odświeża listę wpisów z lokalnej bazy danych.
     *
     * Skanuje katalog danych w poszukiwaniu plików measurements_*.json
     * i wypełnia listę wpisami posortowanymi według daty modyfikacji.
     */
    void refresh();

signals:
    /**
     * @brief Emitowany gdy użytkownik kliknie wpis historii.
     *
     * Wywołuje załadowanie danych na wykres w trybie historycznym.
     * @param m Pomiary wczytane z lokalnej bazy.
     */
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
