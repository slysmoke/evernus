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
#include <QStringBuilder>
#include <QFileInfo>
#include <QSettings>
#include <QRegExp>
#include <QFile>
#include <QDir>

#include "PathSettings.h"
#include "PathUtils.h"

#include "MarketLogExternalOrderImporterThread.h"

namespace Evernus
{
    void MarketLogExternalOrderImporterThread::run()
    {
        const auto logPath = PathUtils::getMarketLogsPath();
        if (logPath.isEmpty())
        {
            emit error(tr("Could not determine market log path!"));
            return;
        }

        const QDir basePath{logPath};
        const auto files = basePath.entryList(QStringList{"*.txt"}, QDir::Files | QDir::Readable);

        ExternalOrderList result;

        QSettings settings;
        const auto deleteLogs = settings.value(PathSettings::deleteLogsKey, PathSettings::deleteLogsDefault).toBool();

        const QRegExp charLogWildcard{
            settings.value(PathSettings::characterLogWildcardKey, PathSettings::characterLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};
        const QRegExp corpLogWildcard{
            settings.value(PathSettings::corporationLogWildcardKey, PathSettings::corporationLogWildcardDefault).toString(),
            Qt::CaseInsensitive,
            QRegExp::Wildcard};

        LogTimeMap timeMap;

        for (const auto &file : files)
        {
            if (isInterruptionRequested())
                break;

            if (charLogWildcard.exactMatch(file) || corpLogWildcard.exactMatch(file))
                continue;

            getExternalOrder(logPath % "/" % file, result, deleteLogs, timeMap);
        }

        emit finished(result);
    }

    void MarketLogExternalOrderImporterThread::getExternalOrder(const QString &logPath, ExternalOrderList &orders, bool deleteLog, LogTimeMap &timeMap)
    {
        QFile file{logPath};
        if (!file.open(QIODevice::ReadOnly))
            return;

        file.readLine();

        QFileInfo info{file};

        const auto priceTime = info.created().toUTC();
        const auto logColumns = 14;

        while (!file.atEnd())
        {
            const QString line = file.readLine();
            const auto values = line.split(',');

            if (values.count() >= logColumns)
            {
                auto order = ExternalOrder::parseLogLine(values);
                if (order.getId() == ExternalOrder::invalidId)
                    continue;

                const auto it = timeMap.find(order.getTypeId());
                if (it != std::end(timeMap) && priceTime < it->second)
                    return;

                timeMap[order.getTypeId()] = priceTime;

                order.setUpdateTime(priceTime);

                orders.emplace_back(std::move(order));
            }
        }

        if (deleteLog)
            file.remove();
    }
}
