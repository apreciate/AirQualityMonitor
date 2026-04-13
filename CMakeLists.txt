cmake_minimum_required(VERSION 3.16)
project(AirQualityMonitor VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Qt6 (fallback to Qt5)
find_package(Qt6 COMPONENTS Core Widgets Network Charts QUIET)
if(NOT Qt6_FOUND)
    find_package(Qt5 COMPONENTS Core Widgets Network Charts REQUIRED)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

# nlohmann/json (header-only, bundled)
include_directories(${CMAKE_SOURCE_DIR}/include)

# GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Sources
set(SOURCES
    src/main.cpp
    src/api/GiosApiClient.cpp
    src/database/LocalDatabase.cpp
    src/models/Station.cpp
    src/models/Sensor.cpp
    src/models/Measurement.cpp
    src/models/AirQualityIndex.cpp
    src/analysis/DataAnalyzer.cpp
    src/gui/MainWindow.cpp
    src/gui/StationListWidget.cpp
    src/gui/ChartWidget.cpp
    src/gui/AnalysisWidget.cpp
    src/gui/MapWidget.cpp
)

set(HEADERS
    src/api/GiosApiClient.h
    src/database/LocalDatabase.h
    src/models/Station.h
    src/models/Sensor.h
    src/models/Measurement.h
    src/models/AirQualityIndex.h
    src/analysis/DataAnalyzer.h
    src/gui/MainWindow.h
    src/gui/StationListWidget.h
    src/gui/ChartWidget.h
    src/gui/AnalysisWidget.h
    src/gui/MapWidget.h
)

# Main executable
add_executable(AirQualityMonitor ${SOURCES} ${HEADERS})

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(AirQualityMonitor
        Qt6::Core Qt6::Widgets Qt6::Network Qt6::Charts
    )
else()
    target_link_libraries(AirQualityMonitor
        Qt5::Core Qt5::Widgets Qt5::Network Qt5::Charts
    )
endif()

# Tests
enable_testing()

set(TEST_SOURCES
    tests/test_models.cpp
    tests/test_analysis.cpp
    tests/test_database.cpp
    src/models/Station.cpp
    src/models/Sensor.cpp
    src/models/Measurement.cpp
    src/models/AirQualityIndex.cpp
    src/analysis/DataAnalyzer.cpp
    src/database/LocalDatabase.cpp
)

add_executable(AirQualityMonitorTests ${TEST_SOURCES})
target_include_directories(AirQualityMonitorTests PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/include
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(AirQualityMonitorTests
        GTest::gtest_main Qt6::Core Qt6::Network
    )
else()
    target_link_libraries(AirQualityMonitorTests
        GTest::gtest_main Qt5::Core Qt5::Network
    )
endif()

# Skopiuj DLL Qt obok exe testów (Windows)
if(WIN32)
    add_custom_command(TARGET AirQualityMonitorTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Core>
            $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Network>
            $<TARGET_FILE_DIR:AirQualityMonitorTests>
    )
endif()

include(GoogleTest)
gtest_discover_tests(AirQualityMonitorTests)

# Doxygen
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating documentation with Doxygen"
        VERBATIM
    )
endif()
