#include <gtest/gtest.h>
#include "../src/analysis/DataAnalyzer.h"
#include "../src/models/Measurement.h"

// Pomocnik: buduje Measurement z wektora wartości (co godzinę od bazy)
static Measurement makeMeasurement(const std::vector<std::optional<double>> &vals)
{
    Measurement m;
    m.sensorId = 1;
    m.key = "TEST";
    QDateTime base = QDateTime::fromString("2024-01-01 00:00:00",
                                           "yyyy-MM-dd HH:mm:ss");
    for (int i = 0; i < (int)vals.size(); ++i) {
        MeasurementPoint p;
        p.date  = base.addSecs(i * 3600);
        p.value = vals[i];
        m.values.push_back(p);
    }
    return m;
}

// ─── ValidPoints ──────────────────────────────────────────────────────────────

TEST(DataAnalyzerTest, ValidPointsSkipsNulls)
{
    auto m = makeMeasurement({1.0, std::nullopt, 3.0, std::nullopt, 5.0});
    auto pts = DataAnalyzer::validPoints(m);
    EXPECT_EQ(pts.size(), 3u);
    EXPECT_NEAR(pts[0].second, 1.0, 1e-9);
    EXPECT_NEAR(pts[1].second, 3.0, 1e-9);
    EXPECT_NEAR(pts[2].second, 5.0, 1e-9);
}

TEST(DataAnalyzerTest, ValidPointsAllNull)
{
    auto m = makeMeasurement({std::nullopt, std::nullopt});
    EXPECT_TRUE(DataAnalyzer::validPoints(m).empty());
}

// ─── Min / Max / Avg ──────────────────────────────────────────────────────────

TEST(DataAnalyzerTest, MinMaxAvg)
{
    auto m = makeMeasurement({10.0, 5.0, 20.0, 15.0});
    auto result = DataAnalyzer::analyze(m);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->minValue, 5.0,  1e-9);
    EXPECT_NEAR(result->maxValue, 20.0, 1e-9);
    EXPECT_NEAR(result->avgValue, 12.5, 1e-9);
    EXPECT_EQ(result->validCount, 4);
    EXPECT_EQ(result->totalCount, 4);
}

TEST(DataAnalyzerTest, AvgIgnoresNulls)
{
    auto m = makeMeasurement({10.0, std::nullopt, 20.0});
    auto result = DataAnalyzer::analyze(m);

    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->avgValue, 15.0, 1e-9);
    EXPECT_EQ(result->validCount, 2);
    EXPECT_EQ(result->totalCount, 3);
}

// ─── Trend ────────────────────────────────────────────────────────────────────

TEST(DataAnalyzerTest, TrendRising)
{
    auto m = makeMeasurement({1.0, 2.0, 3.0, 4.0, 5.0});
    auto result = DataAnalyzer::analyze(m);
    ASSERT_TRUE(result.has_value());
    EXPECT_GT(result->trend, 0.0);
    EXPECT_EQ(result->trendDescription(), "rosnący ↑");
}

TEST(DataAnalyzerTest, TrendFalling)
{
    auto m = makeMeasurement({5.0, 4.0, 3.0, 2.0, 1.0});
    auto result = DataAnalyzer::analyze(m);
    ASSERT_TRUE(result.has_value());
    EXPECT_LT(result->trend, 0.0);
    EXPECT_EQ(result->trendDescription(), "malejący ↓");
}

TEST(DataAnalyzerTest, TrendStable)
{
    auto m = makeMeasurement({5.0, 5.0, 5.0, 5.0});
    auto result = DataAnalyzer::analyze(m);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->trend, 0.0, 1e-9);
    EXPECT_EQ(result->trendDescription(), "stabilny →");
}

TEST(DataAnalyzerTest, EmptyMeasurement)
{
    Measurement m;
    m.sensorId = 1;
    m.key = "TEST";
    auto result = DataAnalyzer::analyze(m);
    EXPECT_FALSE(result.has_value());
}

TEST(DataAnalyzerTest, SinglePoint)
{
    auto m = makeMeasurement({42.0});
    auto result = DataAnalyzer::analyze(m);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->minValue, 42.0, 1e-9);
    EXPECT_NEAR(result->maxValue, 42.0, 1e-9);
    EXPECT_NEAR(result->avgValue, 42.0, 1e-9);
}

// ─── calculateTrend standalone ────────────────────────────────────────────────

TEST(DataAnalyzerTest, CalculateTrendKnownSlope)
{
    // y = 2x -> slope = 2
    std::vector<double> vals = {0, 2, 4, 6, 8};
    double trend = DataAnalyzer::calculateTrend(vals);
    EXPECT_NEAR(trend, 2.0, 1e-6);
}

TEST(DataAnalyzerTest, CalculateTrendSingleValue)
{
    std::vector<double> vals = {99.0};
    EXPECT_NEAR(DataAnalyzer::calculateTrend(vals), 0.0, 1e-9);
}
