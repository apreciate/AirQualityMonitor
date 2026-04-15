================================================================================
  Monitor Jakości Powietrza – GIOŚ
  Projekt zaliczeniowy: Języki Programowania Obiektowego 2026
================================================================================

OPIS PROJEKTU
-------------
Aplikacja desktopowa (C++17 / Qt) do monitorowania jakości powietrza w Polsce.
Pobiera dane z publicznego REST API Głównego Inspektoratu Ochrony Środowiska
(GIOŚ) i prezentuje je w formie wykresów oraz analiz statystycznych.

Dane API: https://powietrze.gios.gov.pl/pjp/content/api


FUNKCJONALNOŚCI
---------------
[✓] Pobieranie danych z API GIOŚ (REST/JSON, asynchronicznie)
[✓] Zapis danych do lokalnej bazy (pliki JSON)
[✓] Odczyt danych historycznych z lokalnej bazy
[✓] Prezentacja danych na wykresie (Qt Charts, wybór zakresu czasu)
[✓] Analiza danych: min, max, średnia, trend (regresja liniowa)
[✓] Filtrowanie stacji po mieście / województwie
[✓] Indeks jakości powietrza z kodowaniem kolorystycznym
[✓] Interaktywna mapa stacji (Leaflet.js przez WebEngine)
[✓] Eksport wykresu do pliku PNG
[✓] Obsługa błędów sieciowych z propozycją danych historycznych
[✓] Interfejs graficzny (Qt Widgets, Fusion style)
[✓] Wielowątkowość: Qt Network działa asynchronicznie (nie blokuje GUI)
[✓] Dokumentacja Doxygen
[✓] Testy jednostkowe (GoogleTest)


WYMAGANIA
---------
  - CMake >= 3.16
  - Qt 5.15+ lub Qt 6.x (z modułami: Core, Widgets, Network, Charts, WebEngine)
  - Kompilator C++17 (GCC 9+, Clang 10+, MSVC 2019+)
  - Graphviz (opcjonalnie, do wykresów Doxygen)
  - Dostęp do Internetu (do pobierania danych z API)

Instalacja Qt i zależności:

  Ubuntu/Debian:
    sudo apt install qt6-base-dev qt6-charts-dev qt6-webengine-dev \
                     cmake build-essential libgtest-dev graphviz

  Fedora/RHEL:
    sudo dnf install qt6-qtbase-devel qt6-qtcharts-devel \
                     qt6-qtwebengine-devel cmake gcc-c++ gtest-devel graphviz

  Windows (vcpkg):
    vcpkg install qt6 gtest

  macOS (Homebrew):
    brew install qt6 googletest graphviz cmake


BUDOWANIE PROJEKTU
------------------
  1. Sklonuj repozytorium:
       git clone https://github.com/<user>/AirQualityMonitor.git
       cd AirQualityMonitor

  2. Utwórz katalog budowania:
       mkdir build && cd build

  3. Skonfiguruj CMake:
       cmake ..
       # lub z explicite wskazanym Qt:
       cmake .. -DCMAKE_PREFIX_PATH=/usr/lib/x86_64-linux-gnu/cmake/Qt6

  4. Zbuduj:
       cmake --build . --parallel

  5. Uruchom:
       ./AirQualityMonitor         # Linux/macOS
       AirQualityMonitor.exe       # Windows


URUCHOMIENIE TESTÓW
-------------------
  W katalogu build:

    ctest --verbose
    # lub bezpośrednio:
    ./AirQualityMonitorTests


GENEROWANIE DOKUMENTACJI
------------------------
  Wymagany Doxygen i Graphviz.

  W katalogu build:
    cmake --build . --target docs

  Dokumentacja zostanie wygenerowana w: build/docs/html/index.html


STRUKTURA PROJEKTU
------------------
  AirQualityMonitor/
  ├── CMakeLists.txt
  ├── README.txt
  ├── docs/
  │   └── Doxyfile.in
  ├── src/
  │   ├── main.cpp
  │   ├── api/
  │   │   ├── GiosApiClient.h       – klient REST API GIOŚ
  │   │   └── GiosApiClient.cpp
  │   ├── models/
  │   │   ├── Station.h/cpp         – model stacji pomiarowej
  │   │   ├── Sensor.h/cpp          – model stanowiska pomiarowego
  │   │   ├── Measurement.h/cpp     – model danych pomiarowych
  │   │   └── AirQualityIndex.h/cpp – model indeksu jakości powietrza
  │   ├── database/
  │   │   ├── LocalDatabase.h/cpp   – lokalna baza danych JSON (Singleton)
  │   ├── analysis/
  │   │   ├── DataAnalyzer.h/cpp    – analiza statystyczna (min/max/avg/trend)
  │   └── gui/
  │       ├── MainWindow.h/cpp      – główne okno (Mediator)
  │       ├── StationListWidget.h/cpp – lista stacji i sensorów
  │       ├── ChartWidget.h/cpp     – wykres (Qt Charts)
  │       ├── AnalysisWidget.h/cpp  – panel analizy statystycznej
  │       └── MapWidget.h/cpp       – mapa (Leaflet.js + WebEngine)
  └── tests/
      ├── test_models.cpp           – testy modeli danych
      ├── test_analysis.cpp         – testy analizatora
      └── test_database.cpp         – testy lokalnej bazy danych


WZORCE PROJEKTOWE
-----------------
  1. Singleton   – LocalDatabase: jeden punkt dostępu do bazy danych
  2. Facade      – GiosApiClient: ukrywa szczegóły HTTP za prostym interfejsem
  3. Mediator    – MainWindow: pośredniczy w komunikacji między widgetami
  4. Observer    – sygnały/sloty Qt (np. GiosApiClient → MainWindow → widgety)
  5. Strategy    – DataAnalyzer: analiza możliwa do rozszerzenia bez modyfikacji GUI


WIELOWĄTKOWOŚĆ
--------------
Pobieranie danych z API odbywa się asynchronicznie przez Qt Network
(QNetworkAccessManager). GUI nie jest blokowane podczas oczekiwania na odpowiedź.
Wyniki trafiają do GUI przez mechanizm sygnałów/slotów Qt (bezpieczny
między wątkami dzięki kolejkowanym połączeniom).


LOKALNA BAZA DANYCH
-------------------
Dane zapisywane są w katalogu:
  Linux:   ~/.local/share/AirQualityMonitor/
  Windows: %APPDATA%\AirQualityMonitor\
  macOS:   ~/Library/Application Support/AirQualityMonitor/

Pliki JSON:
  stations.json             – lista wszystkich stacji
  sensors_<stationId>.json  – sensory dla stacji
  measurements_<sensorId>.json – dane pomiarowe
  aqi_<stationId>.json      – indeks jakości powietrza


UWAGI
-----
  • Karta "Mapa" wymaga połączenia z Internetem (ładuje Leaflet.js z CDN
    oraz kafelki OpenStreetMap).
  • Dane pomiarowe aktualizowane są co ok. godzinę przez API GIOŚ.
  • Wartości null w pomiarach oznaczają brak odczytu w danym czasie.


AUTOR
-----
  Projekt zaliczeniowy – JPO 2025/2026
================================================================================
