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
#include <QStandardPaths>
#include <QSettings>

#include "PathSettings.h"

#include "PathUtils.h"

namespace Evernus
{
    namespace PathUtils
    {
        QString getMarketLogsPath()
        {
            QSettings settings;

            auto logPath = settings.value(PathSettings::marketLogsPathKey).toString();
            if (logPath.isEmpty())
                logPath = QStandardPaths::locate(QStandardPaths::DocumentsLocation, QStringLiteral("EVE/logs/Marketlogs"), QStandardPaths::LocateDirectory);

            return logPath;
        }
    }
}
