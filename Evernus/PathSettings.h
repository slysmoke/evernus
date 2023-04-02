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
#pragma once

#include <QString>

namespace Evernus
{
    namespace PathSettings
    {
        const auto characterLogWildcardDefault = QStringLiteral("My Orders-*.txt");
        const auto corporationLogWildcardDefault = QStringLiteral("Corporation Orders-*.txt");
        const auto deleteLogsDefault = true;

        const auto marketLogsPathKey = QStringLiteral("path/marketLogs/path");
        const auto deleteLogsKey = QStringLiteral("path/marketLogs/delete");
        const auto characterLogWildcardKey = QStringLiteral("path/marketLogs/characterWildcard");
        const auto corporationLogWildcardKey = QStringLiteral("path/marketLogs/corporationWildcard");
    }
}
