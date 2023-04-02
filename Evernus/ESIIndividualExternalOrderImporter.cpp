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
#include <unordered_set>

#include <QSettings>
#include <QtDebug>

#include <boost/scope_exit.hpp>

#include "EveDataProvider.h"
#include "OrderSettings.h"

#include "ESIIndividualExternalOrderImporter.h"

namespace Evernus
{
    ESIIndividualExternalOrderImporter
    ::ESIIndividualExternalOrderImporter(const EveDataProvider &dataProvider,
                                         ESIInterfaceManager &interfaceManager,
                                         QObject *parent)
        : ESIExternalOrderImporter{dataProvider, interfaceManager, parent}
        , mDataProvider{dataProvider}
    {
    }

    void ESIIndividualExternalOrderImporter::fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const
    {
        if (target.empty())
        {
            emit externalOrdersChanged({}, {});
            return;
        }

        mPreparingRequests = true;
        BOOST_SCOPE_EXIT(this_) {
            this_->mPreparingRequests = false;
        } BOOST_SCOPE_EXIT_END

        mCounter.resetBatchIfEmpty();

        QSettings settings;
        const auto importCitadels = settings.value(OrderSettings::importFromCitadelsKey, OrderSettings::importFromCitadelsDefault).toBool();

        std::unordered_set<uint> processedCitadelRegions;

        for (const auto &pair : target)
        {
            const auto regionId = mDataProvider.getStationRegionId(pair.second);
            if (regionId != 0)
            {
                mCounter.incCount();
                mManager.fetchMarketOrders(regionId, pair.first, [this](auto &&orders, const auto &error, const auto &expires) {
                    Q_UNUSED(expires);
                    processResult(std::move(orders), error);
                });

                if (importCitadels && processedCitadelRegions.find(regionId) == std::end(processedCitadelRegions))
                {
                    processedCitadelRegions.emplace(regionId);

                    const auto citadels = mDataProvider.getCitadelsForRegion(regionId);
                    for (const auto &citadel : citadels)
                    {
                        Q_ASSERT(citadel);

                        if (!citadel->canImportMarket())
                            continue;

                        mCounter.incCount();
                        mManager.fetchCitadelMarketOrders(citadel->getId(), regionId, id, [=](auto &&orders, const auto &error, const auto &expires) {
                            Q_UNUSED(expires);
                            processResult(std::move(orders), error);
                        });

                        processEvents();
                    }
                }
            }

            processEvents();
        }

        qDebug() << "Making" << mCounter.getCount() << "ESI requests...";

        if (mCounter.isEmpty())
        {
            emit externalOrdersChanged({}, mResult);
            mResult.clear();
        }
    }
}
