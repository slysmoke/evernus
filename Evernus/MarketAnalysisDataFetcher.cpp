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

#include <boost/scope_exit.hpp>

#include <QDateTime>
#include <QSettings>
#include <QtDebug>

#include "EveDataProvider.h"
#include "ImportSettings.h"
#include "OrderSettings.h"
#include "SSOUtils.h"

#include "MarketAnalysisDataFetcher.h"

namespace Evernus
{
    MarketAnalysisDataFetcher::MarketAnalysisDataFetcher(const EveDataProvider &dataProvider,
                                                         ESIInterfaceManager &interfaceManager,
                                                         QObject *parent)
        : QObject{parent}
        , mDataProvider{dataProvider}
        , mESIManager{mDataProvider, interfaceManager}
    {
        connect(&mESIManager, &ESIManager::error, this, &MarketAnalysisDataFetcher::genericError);
    }

    bool MarketAnalysisDataFetcher::hasPendingOrderRequests() const noexcept
    {
        return !mOrderCounter.isEmpty();
    }

    bool MarketAnalysisDataFetcher::hasPendingHistoryRequests() const noexcept
    {
        return !mHistoryCounter.isEmpty();
    }

    void MarketAnalysisDataFetcher::importData(const TypeLocationPairs &pairs,
                                               const TypeLocationPairs &ignored,
                                               Character::IdType charId)
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

        if (mHistoryCounter.isEmpty())
        {
            mHistory = std::make_shared<HistoryResultType::element_type>();
            mHistoryCounter.resetBatch();
        }

        QSettings settings;
        const auto marketImportType = static_cast<ImportSettings::MarketOrderImportType>(
            settings.value(ImportSettings::marketOrderImportTypeKey, static_cast<int>(ImportSettings::marketOrderImportTypeDefault)).toInt());
        auto useWholeMarketImport = marketImportType == ImportSettings::MarketOrderImportType::Whole;

        if (!useWholeMarketImport && marketImportType == ImportSettings::MarketOrderImportType::Auto)
            useWholeMarketImport = SSOUtils::useWholeMarketImport(pairs, mDataProvider);

        if (useWholeMarketImport)
            importWholeMarketData(pairs, ignored);
        else
            importIndividualData(pairs, ignored);

        if (settings.value(OrderSettings::importFromCitadelsKey, OrderSettings::importFromCitadelsDefault).toBool())
            importCitadelData(pairs, ignored, charId);

        qDebug() << "Making" << mOrderCounter.getCount() << mHistoryCounter.getCount() << "order and history requests...";

        emit orderStatusUpdated(tr("Waiting for %1 order server replies...").arg(mOrderCounter.getCount()));
        emit historyStatusUpdated(tr("Waiting for %1 history server replies...").arg(mHistoryCounter.getCount()));

        if (mOrderCounter.isEmpty())
            finishOrderImport();
        if (mHistoryCounter.isEmpty())
            finishHistoryImport();
    }

    void MarketAnalysisDataFetcher::processOrders(std::vector<ExternalOrder> &&orders, const QString &errorText)
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

    void MarketAnalysisDataFetcher
    ::processHistory(uint regionId, EveType::IdType typeId, std::map<QDate, MarketHistoryEntry> &&history, const QString &errorText)
    {
        if (mHistoryCounter.advanceAndCheckBatch())
            emit historyStatusUpdated(tr("Waiting for %1 history server replies...").arg(mHistoryCounter.getCount()));

        qDebug() << mHistoryCounter.getCount() << "history remaining; error:" << errorText;

        if (Q_UNLIKELY(!errorText.isEmpty()))
        {
            mAggregatedHistoryErrors << errorText;

            if (mHistoryCounter.isEmpty())
                finishHistoryImport();

            return;
        }

        (*mHistory)[regionId][typeId] = std::move(history);

        if (mHistoryCounter.isEmpty() && !mPreparingRequests)
            finishHistoryImport();
    }

    void MarketAnalysisDataFetcher::importWholeMarketData(const TypeLocationPairs &pairs,
                                                          const TypeLocationPairs &ignored)
    {
        std::unordered_set<uint> regions;
        for (const auto &pair : pairs)
        {
            if (ignored.find(pair) != std::end(ignored))
                continue;

            mHistoryCounter.incCount();
            mESIManager.fetchMarketHistory(pair.second, pair.first, [=](auto &&history, const auto &error, const auto &expires) {
                Q_UNUSED(expires);
                processHistory(pair.second, pair.first, std::move(history), error);
            });

            regions.insert(pair.second);
            processEvents();
        }

        mOrderCounter.addCount(regions.size());

        for (const auto region : regions)
        {
            mESIManager.fetchMarketOrders(region, [=](auto &&orders, const auto &error, const auto &expires) {
                Q_UNUSED(expires);

                filterOrders(orders, pairs);
                processOrders(std::move(orders), error);
            });

            processEvents();
        }
    }

    void MarketAnalysisDataFetcher::importIndividualData(const TypeLocationPairs &pairs,
                                                         const TypeLocationPairs &ignored)
    {
        for (const auto &pair : pairs)
        {
            if (ignored.find(pair) != std::end(ignored))
                continue;

            mOrderCounter.incCount();
            mHistoryCounter.incCount();

            mESIManager.fetchMarketOrders(pair.second, pair.first, [=](auto &&orders, const auto &error, const auto &expires) {
                Q_UNUSED(expires);
                processOrders(std::move(orders), error);
            });

            mESIManager.fetchMarketHistory(pair.second, pair.first, [=](auto &&history, const auto &error, const auto &expires) {
                Q_UNUSED(expires);
                processHistory(pair.second, pair.first, std::move(history), error);
            });

            processEvents();
        }
    }

    void MarketAnalysisDataFetcher::importCitadelData(const TypeLocationPairs &pairs,
                                                      const TypeLocationPairs &ignored,
                                                      Character::IdType charId)
    {
        std::unordered_set<uint> regions;
        for (const auto &pair : pairs)
        {
            if (ignored.find(pair) != std::end(ignored))
                continue;

            regions.insert(pair.second);
        }

        for (const auto region : regions)
        {
            const auto citadels = mDataProvider.getCitadelsForRegion(region);
            for (const auto &citadel : citadels)
            {
                Q_ASSERT(citadel);

                if (!citadel->canImportMarket())
                    continue;

                mOrderCounter.incCount();
                mESIManager.fetchCitadelMarketOrders(citadel->getId(), region, charId, [=](auto &&orders, const auto &error, const auto &expires) {
                    Q_UNUSED(expires);

                    filterOrders(orders, pairs);
                    processOrders(std::move(orders), error);
                });

                processEvents();
            }
        }
    }

    void MarketAnalysisDataFetcher::finishOrderImport()
    {
        qDebug() << "Finished market order import at" << QDateTime::currentDateTime() << mOrders->size();

        emit orderImportEnded(mOrders, mAggregatedOrderErrors.join("\n"));
        mAggregatedOrderErrors.clear();
    }

    void MarketAnalysisDataFetcher::finishHistoryImport()
    {
        qDebug() << "Finished history import at" << QDateTime::currentDateTime() << mHistory->size();

        emit historyImportEnded(mHistory, mAggregatedHistoryErrors.join("\n"));
        mAggregatedHistoryErrors.clear();
    }

    void MarketAnalysisDataFetcher::processEvents()
    {
        mEventProcessor.processEvents();
    }

    void MarketAnalysisDataFetcher::filterOrders(std::vector<ExternalOrder> &orders, const TypeLocationPairs &pairs)
    {
        orders.erase(std::remove_if(std::begin(orders), std::end(orders), [&](const auto &order) {
            return pairs.find(std::make_pair(order.getTypeId(), order.getRegionId())) == std::end(pairs);
        }), std::end(orders));
    }
}
