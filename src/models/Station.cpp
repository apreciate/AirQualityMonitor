#include "Station.h"
#include <QJsonArray>

Station Station::fromJson(const QJsonObject &json)
{
    Station s;

    s.id   = json["Identyfikator stacji"].toInt();
    s.name = json["Nazwa stacji"].toString();
    s.lat  = json["WGS84 φ N"].toString().toDouble();
    s.lon  = json["WGS84 λ E"].toString().toDouble();
    s.city = json["Nazwa miasta"].toString();
    s.commune  = json["Gmina"].toString();
    s.district = json["Powiat"].toString();
    s.province = json["Województwo"].toString();
    s.street   = json["Ulica"].toString();

    return s;
}

QJsonObject Station::toJson() const
{
    QJsonObject obj;
    obj["id"]       = id;
    obj["name"]     = name;
    obj["lat"]      = lat;
    obj["lon"]      = lon;
    obj["city"]     = city;
    obj["street"]   = street;
    obj["commune"]  = commune;
    obj["district"] = district;
    obj["province"] = province;
    return obj;
}

QString Station::displayName() const
{
    return QString("[%1] %2 – %3").arg(id).arg(name).arg(city);
}