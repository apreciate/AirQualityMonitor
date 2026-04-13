#include <gtest/gtest.h>
#include <QDir>
#include <QStandardPaths>
#include "../src/database/LocalDatabase.h"

// Uwaga: testy używają rzeczywistego LocalDatabase (Singleton).
// Przed testami czyścimy pliki, żeby testy były izolowane.

class DatabaseTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Usuń pliki testowe przed każdym testem
        QString dir = LocalDatabase::instance().dataDirectory();
        QDir(dir).removeRecursively();
        QDir().mkpath(dir);
    }
};

// ─── Stacje ───────────────────────────────────────────────────────────────────

TEST_F(DatabaseTest, SaveAndLoadStations)
{
    Station s1, s2;
    s1.id = 1; s1.name = "Stacja A"; s1.city = "Poznań";
    s1.lat = 52.4; s1.lon = 16.9; s1.province = "wielkopolskie";
    s2.id = 2; s2.name = "Stacja B"; s2.city = "Warszawa";
    s2.lat = 52.2; s2.lon = 21.0; s2.province = "mazowieckie";

    LocalDatabase::instance().saveStations({s1, s2});
    auto loaded = LocalDatabase::instance().loadStations();

    ASSERT_EQ(loaded.size(), 2u);
    EXPECT_EQ(loaded[0].id,   1);
    EXPECT_EQ(loaded[0].name, "Stacja A");
    EXPECT_EQ(loaded[1].city, "Warszawa");
    EXPECT_NEAR(loaded[0].lat, 52.4, 1e-6);
}

TEST_F(DatabaseTest, LoadStationsWhenEmpty)
{
    auto loaded = LocalDatabase::instance().loadStations();
    EXPECT_TRUE(loaded.empty());
}

// ─── Sensory ──────────────────────────────────────────────────────────────────

TEST_F(DatabaseTest, SaveAndLoadSensors)
{
    Sensor s;
    s.id = 92; s.stationId = 14;
    s.paramName = "pył PM10"; s.paramFormula = "PM10";
    s.paramCode = "PM10"; s.paramId = 3;

    LocalDatabase::instance().saveSensors(14, {s});
    auto loaded = LocalDatabase::instance().loadSensors(14);

    ASSERT_EQ(loaded.size(), 1u);
    EXPECT_EQ(loaded[0].id,          92);
    EXPECT_EQ(loaded[0].paramFormula,"PM10");
}

TEST_F(DatabaseTest, LoadSensorsForUnknownStation)
{
    auto loaded = LocalDatabase::instance().loadSensors(9999);
    EXPECT_TRUE(loaded.empty());
}

// ─── Pomiary ──────────────────────────────────────────────────────────────────

TEST_F(DatabaseTest, SaveAndLoadMeasurement)
{
    Measurement m;
    m.sensorId = 42;
    m.key = "PM2.5";

    MeasurementPoint p1, p2;
    p1.date  = QDateTime::fromString("2024-03-01 12:00:00", "yyyy-MM-dd HH:mm:ss");
    p1.value = 25.3;
    p2.date  = QDateTime::fromString("2024-03-01 13:00:00", "yyyy-MM-dd HH:mm:ss");
    p2.value = std::nullopt;
    m.values = {p1, p2};

    LocalDatabase::instance().saveMeasurement(m);
    auto loaded = LocalDatabase::instance().loadMeasurement(42);

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->sensorId, 42);
    EXPECT_EQ(loaded->key, "PM2.5");
    EXPECT_EQ(loaded->values.size(), 2u);
    ASSERT_TRUE(loaded->values[0].value.has_value());
    EXPECT_NEAR(loaded->values[0].value.value(), 25.3, 1e-6);
    EXPECT_FALSE(loaded->values[1].value.has_value());
}

TEST_F(DatabaseTest, LoadMeasurementNotExists)
{
    auto loaded = LocalDatabase::instance().loadMeasurement(9999);
    EXPECT_FALSE(loaded.has_value());
}

// ─── Indeks jakości ───────────────────────────────────────────────────────────

TEST_F(DatabaseTest, SaveAndLoadAqi)
{
    AirQualityIndex aqi;
    aqi.stationId = 117;
    aqi.calcDate  = QDateTime::fromString("2024-03-01 14:00:00",
                                          "yyyy-MM-dd HH:mm:ss");
    aqi.stationIndex = {2, "Umiarkowany"};

    LocalDatabase::instance().saveAirQualityIndex(aqi);
    auto loaded = LocalDatabase::instance().loadAirQualityIndex(117);

    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->stationId,         117);
    EXPECT_EQ(loaded->stationIndex.id,   2);
    EXPECT_EQ(loaded->stationIndex.name, "Umiarkowany");
}

TEST_F(DatabaseTest, LoadAqiNotExists)
{
    auto loaded = LocalDatabase::instance().loadAirQualityIndex(9999);
    EXPECT_FALSE(loaded.has_value());
}
