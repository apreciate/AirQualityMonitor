#include <QApplication>
#include <QStyleFactory>
#include <QFile>
#include <QSplashScreen>
#include <QPixmap>
#include <QTimer>
#include "gui/MainWindow.h"

/**
 * @brief Punkt wejścia aplikacji.
 *
 * Inicjalizuje środowisko Qt, ustawia styl aplikacji
 * i uruchamia główne okno.
 *
 * @param argc Liczba argumentów wiersza poleceń.
 * @param argv Tablica argumentów wiersza poleceń.
 * @return Kod wyjścia aplikacji.
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AirQualityMonitor");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("JPO2025");

    // Styl Fusion – spójny wygląd na Windows/Linux/macOS
    app.setStyle(QStyleFactory::create("Fusion"));

    // Subtelna paleta kolorów - WYMUSZONY JASNY MOTYW
    QPalette palette;
    palette.setColor(QPalette::Window,          QColor(245, 245, 248));
    palette.setColor(QPalette::WindowText,      QColor(30,  30,  30));
    palette.setColor(QPalette::Base,            QColor(255, 255, 255));
    palette.setColor(QPalette::AlternateBase,   QColor(235, 238, 243));

    // DODANE: Kolor tekstu wewnątrz list, tabel i pól wprowadzania
    palette.setColor(QPalette::Text,            QColor(30,  30,  30));
    palette.setColor(QPalette::PlaceholderText, QColor(120, 120, 120));

    // DODANE: Zabezpieczenie tooltipów przed trybem ciemnym
    palette.setColor(QPalette::ToolTipBase,     QColor(255, 255, 255));
    palette.setColor(QPalette::ToolTipText,     QColor(30,  30,  30));

    palette.setColor(QPalette::Highlight,       QColor(33,  150, 243));
    palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    palette.setColor(QPalette::Button,          QColor(230, 233, 238));
    palette.setColor(QPalette::ButtonText,      QColor(30,  30,  30));

    // Zablokowanie kolorów wyłączonych elementów (żeby nie zlały się z tłem)
    palette.setColor(QPalette::Disabled, QPalette::Text, QColor(150, 150, 150));
    palette.setColor(QPalette::Disabled, QPalette::ButtonText, QColor(150, 150, 150));
    palette.setColor(QPalette::Disabled, QPalette::WindowText, QColor(150, 150, 150));

    app.setPalette(palette);

    MainWindow window;
    window.show();

    // USUNIĘTO: app.setStyleSheet(...) - paleta załatwia sprawę natywnie

    return app.exec();
}