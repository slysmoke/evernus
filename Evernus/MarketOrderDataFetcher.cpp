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
#include <algorithm>

#include <boost/scope_exit.hpp>

#include <QDateTime>
#include <QSettings>
#include <QtDebug>

#include "EveDataProvider.h"
#include "ImportSettings.h"
#include "OrderSettings.h"
#include "SSOUtils.h"

#include "MarketOrderDataFetcher.h"

namespace Evernus
{
    MarketOrderDataFetcher::MarketOrderDataFetcher(const EveDataProvider &dataProvider,
                                                   ESIInterfaceManager &interfaceManager,
                                                   QObject *parent)
        : QObject{parent}
        , mDataProvider{dataProvider}
        , mESIManager{mDataProvider, interfaceManager}
    {
        connect(&mESIManager, &ESIManager::error, this, &MarketOrderDataFetcher::genericError);
    }

    bool MarketOrderDataFetcher::hasPendingOrderRequests() const noexcept
    {
        return !mOrderCounter.isEmpty();
    }

    void MarketOrderDataFetcher::importData(const TypeLocationPairs &pairs, Character::IdType charId)
    {
        mPreparingRequests = true;
        BOOST_SCOPE_EXIT(this_) {
            this_->mPreparingRequests = false;
        } BOOST_SCOPE_EXIT_END

        if (mOrderCounter.isEmpty())
        {
            mOrders = std::make_shared<OrderResultType::element_type>();
            mOrderCounter.resetBatch();
        }

        QSettings settings;
        const auto marketImportType = static_cast<ImportSettings::MarketOrderImportType>(
            settings.value(ImportSettings::marketOrderImportTypeKey, static_cast<int>(ImportSettings::marketOrderImportTypeDefault)).toInt());
        auto useWholeMarketImport = marketImportType == ImportSettings::MarketOrderImportType::Whole;

        if (!useWholeMarketImport && marketImportType == ImportSettings::MarketOrderImportType::Auto)
            useWholeMarketImport = SSOUtils::useWholeMarketImport(pairs, mDataProvider);

        if (useWholeMarketImport)
            importWholeMarketData(pairs);
        else
            importIndividualData(pairs);

        if (settings.value(OrderSettings::importFromCitadelsKey, OrderSettings::importFromCitadelsDefault).toBool())
            importCitadelData(pairs, charId);

        qDebug() << "Making" << mOrderCounter.getCount() << "order requests...";

        emit orderStatusUpdated(tr("Waiting for %1 order server replies...").arg(mOrderCounter.getCount()));

        if (mOrderCounter.isEmpty())
            finishOrderImport();
    }

    void MarketOrderDataFetcher::processOrders(std::vector<ExternalOrder> &&orders, const QString &errorText)
    {
        if (mOrderCounter.advanceAndCheckBatch())
            emit orderStatusUpdated(tr("Waiting for %1 order server replies...").arg(mOrderCounter.getCount()));

        qDebug() << mOrderCounter.getCount() << "orders remaining; error:" << errorText;

        if (Q_UNLIKELY(!errorText.isEmpty()))
        {
            mAggregatedOrderErrors << errorText;

            if (mOrderCounter.isEmpty())
                finishOrderImport();

            return;
        }

        mOrders->reserve(mOrders->size() + orders.size());
        mOrders->insert(std::end(*mOrders),
                        std::make_move_iterator(std::begin(orders)),
                        std::make_move_iterator(std::end(orders)));

        if (mOrderCounter.isEmpty() && !mPreparingRequests)
            finishOrderImport();
    }

    void MarketOrderDataFetcher::importWholeMarketData(const TypeLocationPairs &pairs)
    {
        std::unordered_set<quint64> regions;
        for (const auto &pair : pairs)
        {
            if (regions.find(pair.second) != std::end(regions))
                continue;

            regions.emplace(pair.second);

            mOrderCounter.incCount();
            mESIManager.fetchMarketOrders(pair.second, [=](auto &&orders, const auto &error, const auto &expires) {
                Q_UNUSED(expires);

                filterOrders(orders, pairs);
                processOrders(std::move(orders), error);
            });

            processEvents();
        }
    }

    void MarketOrderDataFetcher::importIndividualData(const TypeLocationPairs &pairs)
    {
        mOrderCounter.addCount(pairs.size());

        for (const auto &pair : pairs)
        {
            mESIManager.fetchMarketOrders(pair.second, pair.first, [=](auto &&orders, const auto &error, const auto &expires) {
                Q_UNUSED(expires);
                processOrders(std::move(orders), error);
            });

            processEvents();
        }
    }

    void MarketOrderDataFetcher::importCitadelData(const TypeLocationPairs &pairs, Character::IdType charId)
    {
        std::unordered_set<quint64> regions;
        for (const auto &pair : pairs)
        {
            if (regions.find(pair.second) != std::end(regions))
                continue;

            regions.emplace(pair.second);

            const auto citadels = mDataProvider.getCitadelsForRegion(pair.second);
            for (const auto &citadel : citadels)
            {
                Q_ASSERT(citadel);

                if (!citadel->canImportMarket())
                    continue;

                mOrderCounter.incCount();
                mESIManager.fetchCitadelMarketOrders(citadel->getId(), pair.second, charId, [=](auto &&orders, const auto &error, const auto &expires) {
                    Q_UNUSED(expires);

                    filterOrders(orders, pairs);
                    processOrders(std::move(orders), error);
                });

                processEvents();
            }
        }
    }

    void MarketOrderDataFetcher::finishOrderImport()
    {
        qDebug() << "Finished market order import at" << QDateTime::currentDateTime() << mOrders->size();

        emit orderImportEnded(mOrders, mAggregatedOrderErrors.join(QStringLiteral("\n")));
        mAggregatedOrderErrors.clear();
    }

    void MarketOrderDataFetcher::processEvents()
    {
        mEventProcessor.processEvents();
    }

    void MarketOrderDataFetcher::filterOrders(std::vector<ExternalOrder> &orders, const TypeLocationPairs &pairs)
    {
        orders.erase(std::remove_if(std::begin(orders), std::end(orders), [&](const auto &order) {
            return pairs.find(std::make_pair(order.getTypeId(), order.getRegionId())) == std::end(pairs);
        }), std::end(orders));
    }
}
