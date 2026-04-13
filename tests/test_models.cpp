#include <gtest/gtest.h>
#include <QJsonObject>
#include <QJsonArray>
#include "../src/models/Station.h"
#include "../src/models/Sensor.h"
#include "../src/models/Measurement.h"
#include "../src/models/AirQualityIndex.h"

// ─── Station ──────────────────────────────────────────────────────────────────

TEST(StationTest, ParseFromJson)
{
    QJsonObject city;
    city["name"] = "Poznań";
    QJsonObject commune;
    commune["communeName"]  = "Poznań";
    commune["districtName"] = "poznański";
    commune["provinceName"] = "wielkopolskie";
    city["commune"] = commune;

    QJsonObject json;
    json["id"]            = 117;
    json["stationName"]   = "Poznań-Polanka";
    json["gegrLat"]       = "52.4016";
    json["gegrLon"]       = "16.9236";
    json["city"]          = city;
    json["addressStreet"] = "Polanka 3";

    Station s = Station::fromJson(json);

    EXPECT_EQ(s.id,       117);
    EXPECT_EQ(s.name,     "Poznań-Polanka");
    EXPECT_NEAR(s.lat,    52.4016, 1e-4);
    EXPECT_NEAR(s.lon,    16.9236, 1e-4);
    EXPECT_EQ(s.city,     "Poznań");
    EXPECT_EQ(s.province, "wielkopolskie");
    EXPECT_EQ(s.street,   "Polanka 3");
}

TEST(StationTest, ToJsonAndBack)
{
    Station s;
    s.id       = 42;
    s.name     = "Test Station";
    s.lat      = 50.0;
    s.lon      = 20.0;
    s.city     = "Warszawa";
    s.province = "mazowieckie";

    QJsonObject json = s.toJson();
    EXPECT_EQ(json["id"].toInt(),       42);
    EXPECT_EQ(json["name"].toString(),  "Test Station");
    EXPECT_NEAR(json["lat"].toDouble(), 50.0, 1e-6);
}

TEST(StationTest, DisplayName)
{
    Station s;
    s.id   = 1;
    s.name = "Stacja ABC";
    s.city = "Kraków";
    EXPECT_TRUE(s.displayName().contains("Stacja ABC"));
    EXPECT_TRUE(s.displayName().contains("Kraków"));
}

// ─── Sensor ───────────────────────────────────────────────────────────────────

TEST(SensorTest, ParseFromJson)
{
    QJsonObject param;
    param["paramName"]    = "pył zawieszony PM10";
    param["paramFormula"] = "PM10";
    param["paramCode"]    = "PM10";
    param["idParam"]      = 3;

    QJsonObject json;
    json["id"]        = 92;
    json["stationId"] = 14;
    json["param"]     = param;

    Sensor sensor = Sensor::fromJson(json);

    EXPECT_EQ(sensor.id,          92);
    EXPECT_EQ(sensor.stationId,   14);
    EXPECT_EQ(sensor.paramFormula,"PM10");
    EXPECT_EQ(sensor.paramId,     3);
}

TEST(SensorTest, DisplayName)
{
    Sensor s;
    s.id          = 5;
    s.paramName   = "dwutlenek azotu";
    s.paramFormula= "NO2";
    EXPECT_TRUE(s.displayName().contains("NO2"));
}

// ─── Measurement ──────────────────────────────────────────────────────────────

TEST(MeasurementTest, ParseFromJson)
{
    QJsonArray values;
    QJsonObject p1, p2;
    p1["date"]  = "2024-03-01 12:00:00";
    p1["value"] = 35.5;
    p2["date"]  = "2024-03-01 13:00:00";
    p2["value"] = QJsonValue::Null;
    values.append(p1);
    values.append(p2);

    QJsonObject json;
    json["key"]    = "PM10";
    json["values"] = values;

    Measurement m = Measurement::fromJson(99, json);

    EXPECT_EQ(m.sensorId,      99);
    EXPECT_EQ(m.key,           "PM10");
    EXPECT_EQ(m.values.size(), 2u);
    EXPECT_TRUE(m.values[0].value.has_value());
    EXPECT_NEAR(m.values[0].value.value(), 35.5, 1e-6);
    EXPECT_FALSE(m.values[1].value.has_value());
}

TEST(MeasurementTest, FilteredByRange)
{
    Measurement m;
    m.sensorId = 1;
    m.key = "PM10";

    auto addPoint = [&](const QString &dateStr, double val) {
        MeasurementPoint p;
        p.date  = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
        p.value = val;
        m.values.push_back(p);
    };

    addPoint("2024-01-01 10:00:00", 10.0);
    addPoint("2024-01-02 10:00:00", 20.0);
    addPoint("2024-01-03 10:00:00", 30.0);

    QDateTime from = QDateTime::fromString("2024-01-01 12:00:00", "yyyy-MM-dd HH:mm:ss");
    QDateTime to   = QDateTime::fromString("2024-01-02 23:00:00", "yyyy-MM-dd HH:mm:ss");

    Measurement filtered = m.filtered(from, to);
    EXPECT_EQ(filtered.values.size(), 1u);
    EXPECT_NEAR(filtered.values[0].value.value(), 20.0, 1e-6);
}

// ─── AirQualityIndex ──────────────────────────────────────────────────────────

TEST(AirQualityIndexTest, ParseFromJson)
{
    QJsonObject level;
    level["id"]             = 2;
    level["indexLevelName"] = "Umiarkowany";

    QJsonObject json;
    json["id"]               = 117;
    json["stCalcDate"]       = "2024-03-01 14:00:00";
    json["stSourceDataDate"] = "2024-03-01 13:00:00";
    json["stIndexLevel"]     = level;

    AirQualityIndex idx = AirQualityIndex::fromJson(json);

    EXPECT_EQ(idx.stationId,          117);
    EXPECT_EQ(idx.stationIndex.id,    2);
    EXPECT_EQ(idx.stationIndex.name,  "Umiarkowany");
    EXPECT_EQ(idx.stationIndex.color(),"#FFFF00");
}

TEST(IndexLevelTest, Colors)
{
    IndexLevel l;
    l.id = 0; EXPECT_EQ(l.color(), "#00E400");
    l.id = 3; EXPECT_EQ(l.color(), "#FF7E00");
    l.id = 5; EXPECT_EQ(l.color(), "#99004C");
    l.id = -1; EXPECT_EQ(l.color(), "#808080");
}
