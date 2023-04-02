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
#include <type_traits>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <set>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/throw_exception.hpp>
#include <boost/scope_exit.hpp>

#include <QtConcurrent>

#include <QCoreApplication>
#include <QSettings>
#include <QtDebug>

#include "MarketAnalysisSettings.h"
#include "EveDataProvider.h"
#include "ArbitrageUtils.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"

#include "OreReprocessingArbitrageModel.h"

namespace Evernus
{
    OreReprocessingArbitrageModel::OreReprocessingArbitrageModel(const EveDataProvider &dataProvider, QObject *parent)
        : ReprocessingArbitrageModel{dataProvider, parent}
    {
        insertSkillMapping(QStringLiteral("Arkonor"), &CharacterData::ReprocessingSkills::mArkonorProcessing);
        insertSkillMapping(QStringLiteral("Bistot"), &CharacterData::ReprocessingSkills::mBistotProcessing);
        insertSkillMapping(QStringLiteral("Crokite"), &CharacterData::ReprocessingSkills::mCrokiteProcessing);
        insertSkillMapping(QStringLiteral("Dark Ochre"), &CharacterData::ReprocessingSkills::mDarkOchreProcessing);
        insertSkillMapping(QStringLiteral("Gneiss"), &CharacterData::ReprocessingSkills::mGneissProcessing);
        insertSkillMapping(QStringLiteral("Hedbergite"), &CharacterData::ReprocessingSkills::mHedbergiteProcessing);
        insertSkillMapping(QStringLiteral("Hemorphite"), &CharacterData::ReprocessingSkills::mHemorphiteProcessing);
        insertSkillMapping(QStringLiteral("Ice"), &CharacterData::ReprocessingSkills::mIceProcessing);
        insertSkillMapping(QStringLiteral("Jaspet"), &CharacterData::ReprocessingSkills::mJaspetProcessing);
        insertSkillMapping(QStringLiteral("Kernite"), &CharacterData::ReprocessingSkills::mKerniteProcessing);
        insertSkillMapping(QStringLiteral("Mercoxit"), &CharacterData::ReprocessingSkills::mMercoxitProcessing);
        insertSkillMapping(QStringLiteral("Omber"), &CharacterData::ReprocessingSkills::mOmberProcessing);
        insertSkillMapping(QStringLiteral("Plagioclase"), &CharacterData::ReprocessingSkills::mPlagioclaseProcessing);
        insertSkillMapping(QStringLiteral("Pyroxeres"), &CharacterData::ReprocessingSkills::mPyroxeresProcessing);
        insertSkillMapping(QStringLiteral("Scordite"), &CharacterData::ReprocessingSkills::mScorditeProcessing);
        insertSkillMapping(QStringLiteral("Spodumain"), &CharacterData::ReprocessingSkills::mSpodumainProcessing);
        insertSkillMapping(QStringLiteral("Veldspar"), &CharacterData::ReprocessingSkills::mVeldsparProcessing);
    }

    void OreReprocessingArbitrageModel::setOrderData(const std::vector<ExternalOrder> &orders,
                                                     PriceType dstPriceType,
                                                     const RegionList &srcRegions,
                                                     const RegionList &dstRegions,
                                                     quint64 srcStation,
                                                     quint64 dstStation,
                                                     bool useStationTax,
                                                     bool ignoreMinVolume,
                                                     bool onlyHighSec,
                                                     double baseYield,
                                                     double sellVolumeLimit,
                                                     const std::optional<double> &customStationTax)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        if (!mCharacter)
            return;

        const auto &reprocessingInfo = mDataProvider.getOreReprocessingInfo();
        const auto reprocessingSkills = mCharacter->getReprocessingSkills();
        const auto reprocessingYield = baseYield *
                                       (1 + reprocessingSkills.mReprocessing * 0.03) *
                                       (1 + reprocessingSkills.mReprocessingEfficiency * 0.02) *
                                       (1 + mCharacter->getReprocessingImplantBonus() / 100.);

        const auto dstSystem = (dstStation == 0) ? (0u) : (mDataProvider.getStationSolarSystemId(dstStation));

        PriceUtils::Taxes taxes;

        QSettings settings;
        const auto useSkillsForDifference = settings.value(
            MarketAnalysisSettings::useSkillsForDifferenceKey, MarketAnalysisSettings::useSkillsForDifferenceDefault).toBool();

        if (useSkillsForDifference)
            taxes = PriceUtils::calculateTaxes(*mCharacter);

        const auto stationTax = (customStationTax) ? (*customStationTax) : (ArbitrageUtils::getStationTax(mCharacter->getCorpStanding()));

        // gather src/dst orders for reprocessing types
        std::unordered_set<EveType::IdType> oreTypes, materialTypes;
        for (const auto &info : reprocessingInfo)
        {
            oreTypes.emplace(info.first);
            for (const auto &material : info.second.mMaterials)
                materialTypes.emplace(material.mMaterialId);
        }

        const auto isValidRegion = getValidRegionFilter();
        const auto isValidStation = getValidStationFilter();

        const auto isSrcOrder = [&](const auto &order) {
            return order.getType() == ExternalOrder::Type::Sell &&
                   isValidRegion(srcRegions, order) &&
                   isValidStation(srcStation, order);
        };

        const auto canSellToOrder = [=](const auto &order) {
            return (dstSystem == 0) ||
                   (order.getRange() == ExternalOrder::rangeStation && dstStation == order.getStationId()) ||
                   (mDataProvider.getDistance(dstSystem, order.getSolarSystemId()) <= static_cast<uint>(order.getRange()));
        };

        const auto isDstOrder = [&](const auto &order) {
            return (order.getType() == dstPriceType) &&
                   (isValidRegion(dstRegions, order)) &&
                   (
                       (dstPriceType == PriceType::Sell && isValidStation(dstStation, order)) ||
                       (dstPriceType == PriceType::Buy && canSellToOrder(order))
                   );
        };

        const auto orderFilter = [&](const auto &order) {
            return (isSrcOrder(order) || isDstOrder(order)) &&
                   (!ignoreMinVolume || order.getMinVolume() <= 1) &&
                   (!onlyHighSec || mDataProvider.getSolarSystemSecurityStatus(order.getSolarSystemId()) >= 0.5);
        };

        std::unordered_map<EveType::IdType, std::multiset<ExternalOrder, ExternalOrder::LowToHigh>> sellMap;
        std::unordered_map<EveType::IdType, std::multiset<std::reference_wrapper<const ExternalOrder>, ExternalOrder::HighToLow>> buyMap;
        for (const auto &order : orders | boost::adaptors::filtered(orderFilter))
        {
            const auto typeId = order.getTypeId();
            if (oreTypes.find(order.getTypeId()) != std::end(oreTypes) && isSrcOrder(order))
                sellMap[typeId].emplace(order);
            if (materialTypes.find(order.getTypeId()) != std::end(materialTypes) && isDstOrder(order))
                buyMap[typeId].emplace(std::cref(order));
        }

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        // for given type, try to find arbitrage opportunities from source orders to dst orders
        // we have 2 versions to avoid branching logic - selling to buy orders and using sell orders

        // NOTE: using std::function because QtConcurrent::mapped cannot infer the result type properly
        const std::function<ItemData (const EveDataProvider::ReprocessingMap::value_type &)> findArbitrageForBuy = [&](const auto &reprocessingInfo) {
            Q_ASSERT(dstPriceType == PriceType::Buy);

            qDebug() << "Finding arbitrage opportunities for" << reprocessingInfo.first;

            const auto sellOrderList = sellMap.find(reprocessingInfo.first);
            if (Q_UNLIKELY(sellOrderList == std::end(sellMap)))
                return ItemData{};

            const auto skill = mReprocessingSkillMap.find(reprocessingInfo.second.mGroupId);
            if (Q_UNLIKELY(skill == std::end(mReprocessingSkillMap)))
            {
                qWarning() << "Missing reprocessing skill for" << reprocessingInfo.first;
                return ItemData{};
            }

            // copy buy map locally so we can modify volumes
            std::unordered_map<EveType::IdType, std::vector<ExternalOrder>> localBuyMap;
            for (const auto &material : reprocessingInfo.second.mMaterials)
            {
                const auto buyOrderList = buyMap.find(material.mMaterialId);
                if (Q_UNLIKELY(buyOrderList == std::end(buyMap)))
                    continue;

                localBuyMap.emplace(std::piecewise_construct,
                                    std::forward_as_tuple(material.mMaterialId),
                                    std::forward_as_tuple(std::begin(buyOrderList->second), std::end(buyOrderList->second)));
            }

            const auto requiredVolume = reprocessingInfo.second.mPortionSize;

            quint64 totalVolume = 0u;
            auto totalIncome = 0.;
            auto totalCost = 0.;

            // keep buying and selling until no more orders are left or we stop making profit
            while (true)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                // unsychronized access but that's ok, since only one thread touches given type orders
                const auto bought = ArbitrageUtils::fillOrders(sellOrderList->second, requiredVolume, true);
                if (bought.empty()) // no more volume to buy
                    break;

                auto cost = std::accumulate(std::begin(bought), std::end(bought), 0., [&](auto total, const auto &order) {
                    return order.mVolume * PriceUtils::getBuyPrice(order.mPrice, taxes, false) + total;
                });

                auto income = 0.;

                // try to sell all the refined goods
                for (const auto &material : reprocessingInfo.second.mMaterials)
                {
                    const uint sellVolume = reprocessingYield * (1 + reprocessingSkills.*(skill->second) * 0.02) * material.mQuantity;

                    const auto buyOrderList = localBuyMap.find(material.mMaterialId);
                    if (buyOrderList == std::end(localBuyMap))   // can't sell this one, maybe there's still profit to be made
                        continue;

                    const auto sold = ArbitrageUtils::fillOrders(buyOrderList->second, sellVolume, false);

                    // cannot sell some stuff, so let's advance in hope we turn in a profit from other materials
                    if (sold.empty())
                        continue;

                    income += std::accumulate(std::begin(sold), std::end(sold), 0., [&](auto total, const auto &order) {
                        return order.mVolume * PriceUtils::getSellPrice(order.mPrice, taxes, false) + total;
                    });

                    if (useStationTax)
                        cost += ArbitrageUtils::getReprocessingTax(sold, stationTax, sellVolume);
                }

                if (income > cost)
                {
                    totalIncome += income;
                    totalCost += cost;
                    totalVolume += requiredVolume;
                }
                else
                {
                    // we stopped being profitable
                    break;
                }
            }

            qDebug() << "Done finding arbitrage opportunities for" << reprocessingInfo.first;

            // discard unprofitable
            if (totalCost >= totalIncome)
                return ItemData{};

            ItemData data;
            data.mId = reprocessingInfo.first;
            data.mTotalProfit = totalIncome;
            data.mTotalCost = totalCost;
            data.mVolume = totalVolume;

            if (!qFuzzyIsNull(data.mTotalCost))
                data.mMargin = 100. * (data.mTotalProfit - data.mTotalCost) / data.mTotalCost;

            return data;
        };

        const std::function<ItemData (const EveDataProvider::ReprocessingMap::value_type &)> findArbitrageForSell = [&](const auto &reprocessingInfo) {
            Q_ASSERT(dstPriceType == PriceType::Sell);

            qDebug() << "Finding arbitrage opportunities for" << reprocessingInfo.first;

            const auto sellOrderList = sellMap.find(reprocessingInfo.first);
            if (Q_UNLIKELY(sellOrderList == std::end(sellMap)))
                return ItemData{};

            const auto skill = mReprocessingSkillMap.find(reprocessingInfo.second.mGroupId);
            if (Q_UNLIKELY(skill == std::end(mReprocessingSkillMap)))
            {
                qWarning() << "Missing reprocessing skill for" << reprocessingInfo.first;
                return ItemData{};
            }

            struct MaterialData
            {
                double mPrice;
                quint64 mVolume;
            };

            // find dst prices and volumes
            std::unordered_map<EveType::IdType, MaterialData> dstPrices;
            for (const auto &material : reprocessingInfo.second.mMaterials)
            {
                const auto dstOrderList = buyMap.find(material.mMaterialId);
                if (dstOrderList == std::end(buyMap) || dstOrderList->second.empty())   // can't sell this one, maybe there's still profit to be made
                    continue;

                // compute our dst limit order price
                auto &data = dstPrices[material.mMaterialId];
                data.mPrice = std::rbegin(dstOrderList->second)->get().getPrice() - PriceUtils::getPriceDelta();
                data.mVolume = std::accumulate(std::begin(dstOrderList->second),
                                               std::end(dstOrderList->second),
                                               0u,
                                               [](auto total, const auto &order) {
                    return total + order.get().getVolumeRemaining();
                }) * sellVolumeLimit;
            }

            const auto requiredVolume = reprocessingInfo.second.mPortionSize;

            quint64 totalVolume = 0u;
            auto totalIncome = 0.;
            auto totalCost = 0.;

            // keep buying and selling until no more orders are left, volume is exhausted or we stop making profit
            while (true)
            {
                QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

                // unsychronized access but that's ok, since only one thread touches given type orders
                const auto bought = ArbitrageUtils::fillOrders(sellOrderList->second, requiredVolume, true);
                if (bought.empty()) // no more volume to buy
                    break;

                auto cost = std::accumulate(std::begin(bought), std::end(bought), 0., [&](auto total, const auto &order) {
                    return order.mVolume * PriceUtils::getBuyPrice(order.mPrice, taxes, false) + total;
                });

                auto income = 0.;

                // try to sell all the refined goods
                for (const auto &material : reprocessingInfo.second.mMaterials)
                {
                    auto &dstData = dstPrices[material.mMaterialId];

                    const quint64 sellVolume = reprocessingYield * (1 + reprocessingSkills.*(skill->second) * 0.02) * material.mQuantity;
                    const auto amount = std::min(sellVolume, dstData.mVolume);
                    if (amount == 0)
                        continue;

                    dstData.mVolume -= amount;
                    totalVolume += amount;

                    const auto price = dstData.mPrice;

                    income += PriceUtils::getSellPrice(price, taxes) * amount;

                    if (useStationTax)
                        cost += stationTax * price * amount;
                }

                if (income > cost)
                {
                    totalIncome += income;
                    totalCost += cost;
                }
                else
                {
                    // we stopped being profitable
                    break;
                }
            }

            qDebug() << "Done finding arbitrage opportunities for" << reprocessingInfo.first;

            // discard unprofitable
            if (totalCost >= totalIncome)
                return ItemData{};

            ItemData data;
            data.mId = reprocessingInfo.first;
            data.mTotalProfit = totalIncome;
            data.mTotalCost = totalCost;
            data.mVolume = totalVolume;

            if (!qFuzzyIsNull(data.mTotalCost))
                data.mMargin = 100. * (data.mTotalProfit - data.mTotalCost) / data.mTotalCost;

            return data;
        };

        // fill our destination collection
        const auto fillData = [](auto &result, const auto &itemData) {
            if (itemData.mId != EveType::invalidId)
                result.emplace_back(itemData);
        };

        // concurrently check for all arbitrage opportunities
        mData = QtConcurrent::blockingMappedReduced<decltype(mData)>(reprocessingInfo,
                                                                     (dstPriceType == PriceType::Buy) ? (findArbitrageForBuy) : (findArbitrageForSell),
                                                                     fillData);
    }

    void OreReprocessingArbitrageModel::insertSkillMapping(const QString &groupName, int CharacterData::ReprocessingSkills::* skill)
    {
        const auto groupId = mDataProvider.getGroupId(groupName);
        if (groupId == 0)
            BOOST_THROW_EXCEPTION(std::runtime_error{"Cannot find group id for: " + groupName.toStdString()});

        mReprocessingSkillMap[groupId] = skill;
    }
}
