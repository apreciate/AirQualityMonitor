#include "DataAnalyzer.h"
#include <numeric>
#include <algorithm>
#include <cmath>

std::vector<std::pair<QDateTime, double>>
DataAnalyzer::validPoints(const Measurement &m)
{
    std::vector<std::pair<QDateTime, double>> pts;
    for (const auto &p : m.values) {
        if (p.value.has_value())
            pts.emplace_back(p.date, p.value.value());
    }
    return pts;
}

double DataAnalyzer::calculateTrend(const std::vector<double> &values)
{
    if (values.size() < 2) return 0.0;

    int n = static_cast<int>(values.size());
    double sumX = 0, sumY = 0, sumXY = 0, sumX2 = 0;

    for (int i = 0; i < n; ++i) {
        sumX  += i;
        sumY  += values[i];
        sumXY += i * values[i];
        sumX2 += i * i;
    }

    double denom = n * sumX2 - sumX * sumX;
    if (std::abs(denom) < 1e-10) return 0.0;

    return (n * sumXY - sumX * sumY) / denom;
}

std::optional<AnalysisResult> DataAnalyzer::analyze(const Measurement &measurement)
{
    auto pts = validPoints(measurement);
    if (pts.empty()) return std::nullopt;

    AnalysisResult result;
    result.totalCount = static_cast<int>(measurement.values.size());
    result.validCount = static_cast<int>(pts.size());

    // Min & max
    auto [minIt, maxIt] = std::minmax_element(pts.begin(), pts.end(),
        [](const auto &a, const auto &b) { return a.second < b.second; });

    result.minValue = minIt->second;
    result.minDate  = minIt->first;
    result.maxValue = maxIt->second;
    result.maxDate  = maxIt->first;

    // Średnia
    double sum = std::accumulate(pts.begin(), pts.end(), 0.0,
        [](double acc, const auto &p) { return acc + p.second; });
    result.avgValue = sum / pts.size();

    // Trend
    std::vector<double> vals;
    vals.reserve(pts.size());
    for (const auto &p : pts)
        vals.push_back(p.second);
    result.trend = calculateTrend(vals);

    return result;
}

QString AnalysisResult::trendDescription() const
{
    constexpr double THRESHOLD = 0.01;
    if (trend > THRESHOLD)  return "rosnący ↑";
    if (trend < -THRESHOLD) return "malejący ↓";
    return "stabilny →";
}
