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
#include <QtDebug>

#include <QSettings>
#include <QUrlQuery>

#include "EveDataProvider.h"
#include "SSOSettings.h"

#include "SSOUtils.h"

namespace Evernus
{
    namespace SSOUtils
    {
        bool useWholeMarketImport(const TypeLocationPairs &target,
                                  const EveDataProvider &dataProvider)
        {
            std::unordered_set<uint> regions;
            TypeLocationPairs typeRegions;

            for (const auto &pair : target)
            {
                const auto regionId = dataProvider.getStationRegionId(pair.second);

                regions.insert(regionId);
                typeRegions.insert(std::make_pair(pair.first, regionId));
            }

            const auto requestsPerRegion = 30; // assuming 30 requests typical worst case, as with The Forge
            const auto wholeImportScore = regions.size() * requestsPerRegion;

            qDebug() << "Auto importer values:" << wholeImportScore << "vs" << typeRegions.size();
            return wholeImportScore < typeRegions.size();
        }

        void clearRefreshTokens()
        {
            QSettings settings;
            settings.beginGroup(SSOSettings::refreshTokenGroup);
            settings.remove(QStringLiteral(""));
            settings.endGroup();
        }

        QVariantMap parseAuthorizationCode(const QByteArray &rawQuery)
        {
            const QUrlQuery query{QByteArray::fromBase64(rawQuery)};
            const auto items = query.queryItems(QUrl::FullyDecoded);

            QVariantMap mappedQuery;
            for (const auto &item : items)
                mappedQuery[item.first] = item.second;

            return mappedQuery;
        }
    }
}
