#include "Sensor.h"

Sensor Sensor::fromJson(const QJsonObject &json)
{
    Sensor s;

    if (json.contains("id"))
        s.id = json["id"].toInt();
    else if (json.contains("Identyfikator stanowiska"))
        s.id = json["Identyfikator stanowiska"].toInt();

    if (json.contains("stationId"))
        s.stationId = json["stationId"].toInt();
    else if (json.contains("Identyfikator stacji"))
        s.stationId = json["Identyfikator stacji"].toInt();

    // Stare API: param.paramName / param.paramFormula
    if (json.contains("param")) {
        QJsonObject param = json["param"].toObject();
        s.paramName    = param["paramName"].toString();
        s.paramFormula = param["paramFormula"].toString();
        s.paramCode    = param["paramCode"].toString();
        s.paramId      = param["idParam"].toInt();
    } else {
        // Nowe API v1
        s.paramName    = json["Wskaźnik"].toString();
        s.paramFormula = json["Wskaźnik - kod"].toString();
        s.paramCode    = json["Wskaźnik - kod"].toString();
        s.paramId      = json["Id wskaźnika"].toInt();
    }

    return s;
}

QJsonObject Sensor::toJson() const
{
    QJsonObject obj;
    obj["id"]          = id;
    obj["stationId"]   = stationId;
    obj["paramName"]   = paramName;
    obj["paramFormula"]= paramFormula;
    obj["paramCode"]   = paramCode;
    obj["paramId"]     = paramId;
    return obj;
}

QString Sensor::displayName() const
{
    return QString("[%1] %2 (%3)").arg(id).arg(paramName).arg(paramFormula);
}