/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QCoreApplication>
#include <QDateTime>
#include <QSettings>
#include <QLocale>
#include <QColor>

#include "PriceSettings.h"
#include "UISettings.h"

#include "TextUtils.h"

namespace Evernus
{
    namespace TextUtils
    {
        QString secondsToString(const std::chrono::seconds &duration)
        {
            QString res;

            const auto minutes = std::chrono::duration_cast<std::chrono::minutes>(duration).count() % 60;
            const auto hours = std::chrono::duration_cast<std::chrono::hours>(duration).count() % 24;
            const auto days = std::chrono::duration_cast<std::chrono::hours>(duration).count() / 24;

            if (hours == 0 && days == 0)
                return res.sprintf(QCoreApplication::translate("TextUtils", "%02dmin").toLatin1().data(), minutes);

            if (days == 0)
                return res.sprintf(QCoreApplication::translate("TextUtils", "%02dh %02dmin").toLatin1().data(), hours, minutes);

            return res.sprintf(QCoreApplication::translate("TextUtils", "%dd %02dh").toLatin1().data(), days, hours);
        }

        QString dateTimeToString(const QDateTime &dt, const QLocale &locale)
        {
            QSettings settings;

            const auto useUTC = settings.value(UISettings::useUTCDatesKey, UISettings::useUTCDatesDefault).toBool();
            return locale.toString((useUTC) ? (dt.toUTC()) : (dt), settings.value(UISettings::dateTimeFormatKey, locale.dateTimeFormat(QLocale::ShortFormat)).toString());
        }

        QString currencyToString(double value, const QLocale &locale)
        {
            QSettings settings;

            const auto omitSymbol
                = settings.value(UISettings::omitCurrencySymbolKey, UISettings::omitCurrencySymbolDefault).toBool();
            return (omitSymbol) ? (locale.toString(value, 'f', 2)) : (locale.toCurrencyString(value, "ISK"));
        }

        QColor getMarginColor(double margin)
        {
            QSettings settings;
            if (margin < settings.value(PriceSettings::minMarginKey, PriceSettings::minMarginDefault).toDouble())
                return QColor{Qt::red};
            if (margin < settings.value(PriceSettings::preferredMarginKey, PriceSettings::preferredMarginDefault).toDouble())
                return QColor{0xff, 0xa5, 0x00};

            return QColor{Qt::green};
        }

        QString getMarginStyleSheet(double margin)
        {
            QSettings settings;
            if (margin < settings.value(PriceSettings::minMarginKey, PriceSettings::minMarginDefault).toDouble())
                return "color: red;";
            if (margin < settings.value(PriceSettings::preferredMarginKey, PriceSettings::preferredMarginDefault).toDouble())
                return "color: orange;";

            return "color: green;";
        }
    }
}
