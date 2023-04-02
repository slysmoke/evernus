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
#include <algorithm>

#include <QSettings>
#include <QtDebug>

#include <boost/scope_exit.hpp>

#include "EveDataProvider.h"
#include "OrderSettings.h"

#include "ESIWholeExternalOrderImporter.h"

namespace Evernus
{
    ESIWholeExternalOrderImporter::ESIWholeExternalOrderImporter(const EveDataProvider &dataProvider,
                                                                 ESIInterfaceManager &interfaceManager,
                                                                 QObject *parent)
        : ESIExternalOrderImporter{dataProvider, interfaceManager, parent}
        , mDataProvider{dataProvider}
    {
    }

    void ESIWholeExternalOrderImporter::fetchExternalOrders(Character::IdType id, const TypeLocationPairs &target) const
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

        mCurrentTarget.clear();
        mCounter.resetBatchIfEmpty();

        std::unordered_set<uint> regions;
        for (const auto &pair : target)
        {
            const auto regionId = mDataProvider.getStationRegionId(pair.second);
            if (regionId != 0)
            {
                regions.insert(regionId);
                mCurrentTarget.insert(std::make_pair(pair.first, regionId));
            }
        }

        mCounter.setCount(regions.size());

        QSettings settings;
        const auto importCitadels = settings.value(OrderSettings::importFromCitadelsKey, OrderSettings::importFromCitadelsDefault).toBool();

        for (const auto region : regions)
        {
            mManager.fetchMarketOrders(region, [=](auto &&orders, const auto &error, const auto &expires) {
                Q_UNUSED(expires);
                processResult(std::move(orders), error);
            });

            if (importCitadels)
            {
                const auto citadels = mDataProvider.getCitadelsForRegion(region);
                for (const auto &citadel : citadels)
                {
                    Q_ASSERT(citadel);

                    if (!citadel->canImportMarket())
                        continue;

                    mCounter.incCount();
                    mManager.fetchCitadelMarketOrders(citadel->getId(), region, id, [=](auto &&orders, const auto &error, const auto &expires) {
                        Q_UNUSED(expires);
                        processResult(std::move(orders), error);
                    });

                    processEvents();
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

    void ESIWholeExternalOrderImporter::filterOrders(std::vector<ExternalOrder> &orders) const
    {
        orders.erase(std::remove_if(std::begin(orders), std::end(orders), [=](const auto &order) {
            return mCurrentTarget.find(std::make_pair(order.getTypeId(), order.getRegionId())) == std::end(mCurrentTarget);
        }), std::end(orders));
    }
}
