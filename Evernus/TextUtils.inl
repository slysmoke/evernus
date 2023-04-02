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
namespace Evernus
{
    namespace TextUtils
    {
        template<class Rep, class Period>
        QString durationToString(std::chrono::duration<Rep, Period> time)
        {
            const auto secs = std::chrono::duration_cast<std::chrono::seconds>(time).count();
            if (secs < 3600)
                return QStringLiteral("%1:%2").arg(secs / 60, 2, 10, QLatin1Char{'0'}).arg(secs % 60, 2, 10, QLatin1Char{'0'});

            return QStringLiteral("%1:%2:%3")
                .arg(secs / 3600, 2, 10, QLatin1Char{'0'})
                .arg((secs % 3600) / 60, 2, 10, QLatin1Char{'0'})
                .arg(secs % 60, 2, 10, QLatin1Char{'0'});
        }
    }
}
