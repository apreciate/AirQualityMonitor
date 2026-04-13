#include "Measurement.h"

// ─── MeasurementPoint ────────────────────────────────────────────────────────

MeasurementPoint MeasurementPoint::fromJson(const QJsonObject &obj)
{
    MeasurementPoint p;
    QString dateStr = obj.contains("date") ? obj["date"].toString()
                                           : obj["Data"].toString();
    // Próbuj różne formaty daty
    p.date = QDateTime::fromString(dateStr, "yyyy-MM-dd HH:mm:ss");
    if (!p.date.isValid())
        p.date = QDateTime::fromString(dateStr, Qt::ISODate);
    if (!p.date.isValid())
        p.date = QDateTime::fromString(dateStr, "yyyy-MM-dd'T'HH:mm:ss");

    QJsonValue val = obj.contains("value") ? obj["value"] : obj["Wartość"];
    if (!val.isNull() && !val.isUndefined() && val.toString() != "null")
        p.value = val.toDouble();

    return p;
}

QJsonObject MeasurementPoint::toJson() const
{
    QJsonObject obj;
    obj["date"] = date.toString("yyyy-MM-dd HH:mm:ss");
    if (value.has_value())
        obj["value"] = value.value();
    else
        obj["value"] = QJsonValue::Null;
    return obj;
}

// ─── Measurement ─────────────────────────────────────────────────────────────

Measurement Measurement::fromJson(int sid, const QJsonObject &json)
{
    Measurement m;
    m.sensorId = sid;
    m.key      = json["key"].toString();

    QJsonArray arr = json["values"].toArray();
    for (const QJsonValue &v : arr)
        m.values.push_back(MeasurementPoint::fromJson(v.toObject()));

    return m;
}

QJsonObject Measurement::toJson() const
{
    QJsonObject obj;
    obj["sensorId"] = sensorId;
    obj["key"]      = key;

    QJsonArray arr;
    for (const auto &p : values)
        arr.append(p.toJson());
    obj["values"] = arr;

    return obj;
}

Measurement Measurement::filtered(const QDateTime &from, const QDateTime &to) const
{
    Measurement result;
    result.sensorId = sensorId;
    result.key      = key;
    for (const auto &p : values) {
        if (p.date >= from && p.date <= to)
            result.values.push_back(p);
    }
    return result;
}