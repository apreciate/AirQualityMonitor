#include "AirQualityIndex.h"

// ─── IndexLevel ───────────────────────────────────────────────────────────────

IndexLevel IndexLevel::fromJson(const QJsonObject &json)
{
    IndexLevel l;
    l.id   = json["id"].toInt(-1);
    l.name = json["indexLevelName"].toString("Brak danych");
    return l;
}

QString IndexLevel::color() const
{
    switch (id) {
        case 0: return "#00E400"; // Bardzo dobry – zielony
        case 1: return "#92D050"; // Dobry
        case 2: return "#FFFF00"; // Umiarkowany – żółty
        case 3: return "#FF7E00"; // Dostateczny – pomarańczowy
        case 4: return "#FF0000"; // Zły – czerwony
        case 5: return "#99004C"; // Bardzo zły – ciemnoczerwony
        default: return "#808080"; // Brak danych – szary
    }
}

// ─── AirQualityIndex ──────────────────────────────────────────────────────────

AirQualityIndex AirQualityIndex::fromJson(const QJsonObject &json)
{
    AirQualityIndex idx;
    idx.stationId  = json["id"].toInt();
    idx.calcDate   = QDateTime::fromString(json["stCalcDate"].toString(),
                                           "yyyy-MM-dd HH:mm:ss");
    idx.sourceDate = QDateTime::fromString(json["stSourceDataDate"].toString(),
                                           "yyyy-MM-dd HH:mm:ss");

    if (!json["stIndexLevel"].isNull())
        idx.stationIndex = IndexLevel::fromJson(json["stIndexLevel"].toObject());
    else
        idx.stationIndex = {-1, "Brak danych"};

    return idx;
}

QJsonObject AirQualityIndex::toJson() const
{
    QJsonObject obj;
    obj["stationId"]       = stationId;
    obj["calcDate"]        = calcDate.toString("yyyy-MM-dd HH:mm:ss");
    obj["sourceDate"]      = sourceDate.toString("yyyy-MM-dd HH:mm:ss");
    obj["indexLevelId"]    = stationIndex.id;
    obj["indexLevelName"]  = stationIndex.name;
    return obj;
}
