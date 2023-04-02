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
#include <QJsonDocument>
#include <QJsonObject>

#include "Citadel.h"

#include "CitadelManager.h"

namespace Evernus
{
    void CitadelManager::fetchCitadels(Callback callback) const
    {
        mInterface.fetchCitadels([=, callback = std::move(callback)](const auto &result, const auto &error) {
            if (Q_UNLIKELY(!error.isEmpty()))
            {
                callback({}, error);
                return;
            }

            const auto object = result.object();

            std::vector<Citadel> citadels;
            citadels.reserve(object.size());

            for (auto it = std::begin(object); it != std::end(object); ++it)
            {
                const auto citadelObj = it.value().toObject();

                Citadel citadel{it.key().toULongLong()};
                citadel.setTypeId(citadelObj["typeId"].toInt());
                citadel.setRegionId(citadelObj["regionId"].toInt());
                citadel.setSolarSystemId(citadelObj["systemId"].toInt());
                citadel.setLastSeen(QDateTime::fromString(citadelObj["lastSeen"].toString(), Qt::ISODate));
                citadel.setFirstSeen(QDateTime::fromString(citadelObj["firstSeen"].toString(), Qt::ISODate));
                citadel.setName(citadelObj["name"].toString());
                citadel.setPublic(citadelObj["public"].toBool());

                const auto location = citadelObj["location"].toObject();
                citadel.setX(location["x"].toDouble());
                citadel.setY(location["y"].toDouble());
                citadel.setZ(location["z"].toDouble());

                citadels.emplace_back(std::move(citadel));
            }

            callback(std::move(citadels), QString{});
        });
    }
}
