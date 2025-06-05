#include "util.h"
#include "nc_exception.h"

#include <math.h>

double nucare::math::quadraticEquation(const double a, const double b, const double c)
{
    auto delta = b * b - 4 * a * c;
    if (delta <= 0)
        NC_THROW_ARG_ERROR("Quadratic equation can't be solved.");

    return (-b + sqrt(delta)) / (2 * a);
}

double nucare::math::linear(const double xStart, const double xEnd, const double yStart, const double yEnd, double position)
{
    auto xRange = xEnd - xStart;
    auto yRange = yEnd - yStart;

    return yStart + yRange * (position - xStart) / xRange;
}

QString nucare::toExponentUnicode(const char &c)
{
    switch (c) {
            case '0':
                return QStringLiteral("\u2070"); // ⁰ (SUPERSCRIPT ZERO)
            case '1':
                return QStringLiteral("\u00b9"); // ¹ (SUPERSCRIPT ONE)
            case '2':
                return QStringLiteral("\u00b2"); // ² (SUPERSCRIPT TWO)
            case '3':
                return QStringLiteral("\u00b3"); // ³ (SUPERSCRIPT THREE)
            default:
                return QString(0x2070 + (c - '0'));
        }
}

QString nucare::toExponentFormat(const int &base, const int &exponent)
{
    QString expTxt = QString::number(exponent);
    QStringList ret(QString::number(base));
    for (auto c : expTxt) {
        ret.append(toExponentUnicode(c.toLatin1()));
    }

    return ret.join("");
}

QString datetime::formatDate_yyyyMMdd_HHmm(QDateTime &datetime)
{
    return datetime.toString("yyyyMMdd HH:mm");
}

QString datetime::formatDuration(int totalSeconds) {
    int day = totalSeconds / (24 * 3600);
    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;

    QStringList ret;
    if (day > 0) ret << QString::number(day) << "d";
    if (hours > 0) ret << QString::number(hours) << "hrs";
    if (minutes > 0) ret << QString::number(minutes) << "min";

    return ret.join("");
}

QString datetime::formatIsoDate(nucare::Time &datetime)
{
    return datetime.toString(Qt::ISODateWithMs);
}
