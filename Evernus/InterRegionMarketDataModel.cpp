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
#include <set>

#include <QEventLoop>
#include <QSettings>
#include <QLocale>
#include <QColor>
#include <QIcon>

#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include "MarketAnalysisSettings.h"
#include "EveDataProvider.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"
#include "MathUtils.h"
#include "TextUtils.h"

#include "InterRegionMarketDataModel.h"

using namespace boost::accumulators;

namespace Evernus
{
    InterRegionMarketDataModel::InterRegionMarketDataModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractTableModel{parent}
        , ModelWithTypes{}
        , mDataProvider{dataProvider}
    {
    }

    int InterRegionMarketDataModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant InterRegionMarketDataModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return QVariant{};

        const auto column = index.column();
        const auto &data = mData[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case nameColumn:
                    return mDataProvider.getTypeName(data.mId);
                case scoreColumn:
                    return locale.toString(data.mDifference * data.mVolume, 'f', 0);
                case srcRegionColumn:
                    return mDataProvider.getRegionName(data.mSrcRegion);
                case srcBuyPriceColumn:
                    return TextUtils::currencyToString(data.mSrcBuyPrice, locale);
                case srcSellPriceColumn:
                    return TextUtils::currencyToString(data.mSrcSellPrice, locale);
                case srcOrderCountColumn:
                    return QStringLiteral("%1 / %2").arg(locale.toString(data.mSrcBuyOrderCount)).arg(locale.toString(data.mSrcSellOrderCount));
                case dstRegionColumn:
                    return mDataProvider.getRegionName(data.mDstRegion);
                case dstBuyPriceColumn:
                    return TextUtils::currencyToString(data.mDstBuyPrice, locale);
                case dstSellPriceColumn:
                    return TextUtils::currencyToString(data.mDstSellPrice, locale);
                case dstOrderCountColumn:
                    return QStringLiteral("%1 / %2").arg(locale.toString(data.mDstBuyOrderCount)).arg(locale.toString(data.mDstSellOrderCount));
                case differenceColumn:
                    return TextUtils::currencyToString(data.mDifference, locale);
                case volumeColumn:
                    return locale.toString(data.mVolume);
                case marginColumn:
                    return QStringLiteral("%1%2").arg(locale.toString(data.mMargin, 'f', 2)).arg(locale.percent());
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mId);
            case scoreColumn:
                return data.mDifference * data.mVolume;
            case srcRegionColumn:
                return mDataProvider.getRegionName(data.mSrcRegion);
            case srcBuyPriceColumn:
                return data.mSrcBuyPrice;
            case srcSellPriceColumn:
                return data.mSrcSellPrice;
            case srcOrderCountColumn:
                return (data.mSrcBuyOrderCount + data.mSrcSellOrderCount) / 2.;
            case dstRegionColumn:
                return mDataProvider.getRegionName(data.mDstRegion);
            case dstBuyPriceColumn:
                return data.mDstBuyPrice;
            case dstSellPriceColumn:
                return data.mDstSellPrice;
            case dstOrderCountColumn:
                return (data.mDstBuyOrderCount + data.mDstSellOrderCount) / 2.;
            case differenceColumn:
                return data.mDifference;
            case volumeColumn:
                return data.mVolume;
            case marginColumn:
                return data.mMargin;
            }
            break;
        case Qt::UserRole + 1:
            switch (column) {
            case srcRegionColumn:
                return data.mSrcRegion;
            case dstRegionColumn:
                return data.mDstRegion;
            }
            break;
        case Qt::ToolTipRole:
            if (column == nameColumn)
                return tr("Double-click for detailed market information.");
            break;
        case Qt::DecorationRole:
            if (column == nameColumn)
                return QIcon{":/images/chart_curve.png"};
            break;
        case Qt::ForegroundRole:
            if (column == marginColumn)
                return TextUtils::getMarginColor(data.mMargin);
        }

        return QVariant{};
    }

    QVariant InterRegionMarketDataModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case scoreColumn:
                return tr("Score");
            case srcRegionColumn:
                return tr("Source");
            case srcBuyPriceColumn:
                return tr("5% volume source price (buy)");
            case srcSellPriceColumn:
                return tr("5% volume source price (sell)");
            case srcOrderCountColumn:
                return tr("Source order count (b/s)");
            case dstRegionColumn:
                return tr("Destination");
            case dstBuyPriceColumn:
                return tr("5% volume destination price (buy)");
            case dstSellPriceColumn:
                return tr("5% volume destination price (sell)");
            case dstOrderCountColumn:
                return tr("Destination order count (b/s)");
            case differenceColumn:
                return tr("Best difference");
            case volumeColumn:
                return tr("30-day avg. min. volume");
            case marginColumn:
                return tr("Margin");
            }
        }

        return QVariant{};
    }

    int InterRegionMarketDataModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void InterRegionMarketDataModel::setOrderData(const std::vector<ExternalOrder> &orders,
                                                  const HistoryRegionMap &history,
                                                  quint64 srcStation,
                                                  quint64 dstStation,
                                                  PriceType srcType,
                                                  PriceType dstType)
    {
        beginResetModel();

        mData.clear();

        mSrcPriceType = srcType;
        mDstPriceType = dstType;

        RegionMap<TypeMap<std::multiset<std::reference_wrapper<const ExternalOrder>, ExternalOrder::LowToHigh>>> sellOrders;
        RegionMap<TypeMap<std::multiset<std::reference_wrapper<const ExternalOrder>, ExternalOrder::HighToLow>>> buyOrders;

        RegionMap<TypeMap<quint64>> sellVolumes, buyVolumes;

        QEventLoop loop;

        const auto srcRegionId = (srcStation == 0) ? (0u) : (mDataProvider.getStationRegionId(srcStation));
        const auto dstRegionId = (dstStation == 0) ? (0u) : (mDataProvider.getStationRegionId(dstStation));

        for (const auto &order : orders)
        {
            const auto typeId = order.getTypeId();
            const auto regionId = order.getRegionId();
            const auto stationId = order.getStationId();

            if ((srcRegionId != 0 && srcRegionId == regionId && stationId != srcStation) ||
                (dstRegionId != 0 && dstRegionId == regionId && stationId != dstStation))
            {
                continue;
            }

            if (order.getType() == ExternalOrder::Type::Buy)
            {
                buyOrders[regionId][typeId].insert(std::cref(order));
                buyVolumes[regionId][typeId] += order.getVolumeRemaining();
            }
            else
            {
                sellOrders[regionId][typeId].insert(std::cref(order));
                sellVolumes[regionId][typeId] += order.getVolumeRemaining();
            }

            loop.processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        const auto historyLimit = QDate::currentDate().addDays(-30);

        struct AggrTypeData
        {
            double mBuyPrice = 0.;
            double mSellPrice = 0.;
            quint64 mVolume = 0;
            quint64 mBuyOrderCount = 0;
            quint64 mSellOrderCount = 0;
        };

        RegionMap<TypeMap<AggrTypeData>> aggrTypeData;

        for (const auto regionHistory : history)
        {
            for (const auto type : regionHistory.second)
            {
                AggrTypeData data;

                accumulator_set<double, stats<tag::mean>> priceAcc;

                for (const auto &timePoint : boost::adaptors::reverse(type.second))
                {
                    if (Q_UNLIKELY(timePoint.first < historyLimit))
                        break;

                    data.mVolume += timePoint.second.mVolume;
                    priceAcc(timePoint.second.mAvgPrice);
                }

                const auto avgPrice30 = mean(priceAcc);

                const auto &typeBuyOrders = buyOrders[regionHistory.first][type.first];
                const auto &typeSellOrders = sellOrders[regionHistory.first][type.first];

                data.mVolume /= 30;
                data.mBuyOrderCount = typeBuyOrders.size();
                data.mSellOrderCount = typeSellOrders.size();
                data.mBuyPrice = MathUtils::calcPercentile(typeBuyOrders,
                                                           buyVolumes[regionHistory.first][type.first] * 0.05,
                                                           avgPrice30,
                                                           mDiscardBogusOrders,
                                                           mBogusOrderThreshold);
                data.mSellPrice = MathUtils::calcPercentile(typeSellOrders,
                                                            sellVolumes[regionHistory.first][type.first] * 0.05,
                                                            avgPrice30,
                                                            mDiscardBogusOrders,
                                                            mBogusOrderThreshold);

                aggrTypeData[regionHistory.first].emplace(type.first, std::move(data));

                loop.processEvents(QEventLoop::ExcludeUserInputEvents);
            }
        }

        PriceUtils::Taxes taxes;

        QSettings settings;
        const auto useSkillsForDifference = mCharacter && settings.value(
            MarketAnalysisSettings::useSkillsForDifferenceKey, MarketAnalysisSettings::useSkillsForDifferenceDefault).toBool();

        if (useSkillsForDifference)
            taxes = PriceUtils::calculateTaxes(*mCharacter);

        for (const auto srcRegion : aggrTypeData)
        {
            if (srcRegionId != 0 && srcRegion.first != srcRegionId)
                continue;

            for (const auto type : srcRegion.second)
            {
                for (const auto dstRegion : aggrTypeData)
                {
                    if ((dstRegionId != 0 && dstRegion.first != dstRegionId) || (dstRegion.first == srcRegion.first))
                        continue;

                    const auto dstData = dstRegion.second.find(type.first);
                    if (Q_UNLIKELY(dstData == std::end(dstRegion.second)))
                        continue;

                    // if we're buying from sell orders, we need to either have at least one, or have an average (will be strictly 0.)
                    if (Q_UNLIKELY(srcType == PriceType::Sell && type.second.mBuyPrice == 0.))
                        continue;

                    TypeData data;
                    data.mId = type.first;
                    data.mSrcBuyPrice = type.second.mBuyPrice;
                    data.mSrcSellPrice = type.second.mSellPrice;
                    data.mSrcBuyOrderCount = type.second.mBuyOrderCount;
                    data.mSrcSellOrderCount = type.second.mSellOrderCount;
                    data.mDstBuyPrice = dstData->second.mBuyPrice;
                    data.mDstSellPrice = dstData->second.mSellPrice;
                    data.mDstBuyOrderCount = dstData->second.mBuyOrderCount;
                    data.mDstSellOrderCount = dstData->second.mSellOrderCount;
                    data.mVolume = std::min(type.second.mVolume, dstData->second.mVolume);
                    data.mSrcRegion = srcRegion.first;
                    data.mDstRegion = dstRegion.first;

                    auto realSellPrice = getDstPrice(data);
                    auto realBuyPrice = getSrcPrice(data);

                    if (useSkillsForDifference)
                    {
                        realSellPrice = (mDstPriceType == PriceType::Buy) ? (PriceUtils::getSellPrice(realSellPrice, taxes, false)) : (PriceUtils::getSellPrice(realSellPrice, taxes));
                        realBuyPrice = (mSrcPriceType == PriceType::Buy) ? (PriceUtils::getBuyPrice(realBuyPrice, taxes)) : (PriceUtils::getBuyPrice(realBuyPrice, taxes, false));
                    }

                    data.mDifference = realSellPrice - realBuyPrice;
                    data.mMargin = (qFuzzyIsNull(realSellPrice)) ? (0.) : (100. * data.mDifference / realSellPrice);

                    mData.emplace_back(std::move(data));

                    loop.processEvents(QEventLoop::ExcludeUserInputEvents);
                }
            }
        }

        endResetModel();
    }

    void InterRegionMarketDataModel::setCharacter(const std::shared_ptr<Character> &character)
    {
        beginResetModel();
        mCharacter = character;
        mData.clear();
        endResetModel();
    }

    void InterRegionMarketDataModel::discardBogusOrders(bool flag) noexcept
    {
        mDiscardBogusOrders = flag;
    }

    void InterRegionMarketDataModel::setBogusOrderThreshold(double value) noexcept
    {
        mBogusOrderThreshold = value;
    }

    EveType::IdType InterRegionMarketDataModel::getTypeId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return EveType::invalidId;

        return mData[index.row()].mId;
    }

    Character::IdType InterRegionMarketDataModel::getOwnerId(const QModelIndex &index) const
    {
        return (mCharacter) ? (mCharacter->getId()) : (Character::invalidId);
    }

    uint InterRegionMarketDataModel::getSrcRegionId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        return mData[index.row()].mSrcRegion;
    }

    uint InterRegionMarketDataModel::getDstRegionId(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        return mData[index.row()].mDstRegion;
    }

    void InterRegionMarketDataModel::reset()
    {
        beginResetModel();
        mData.clear();
        endResetModel();
    }

    int InterRegionMarketDataModel::getSrcRegionColumn()
    {
        return srcRegionColumn;
    }

    int InterRegionMarketDataModel::getDstRegionColumn()
    {
        return dstRegionColumn;
    }

    int InterRegionMarketDataModel::getVolumeColumn()
    {
        return volumeColumn;
    }

    int InterRegionMarketDataModel::getMarginColumn()
    {
        return marginColumn;
    }

    double InterRegionMarketDataModel::getSrcPrice(const TypeData &data) const noexcept
    {
        return (mSrcPriceType == PriceType::Buy) ? (data.mSrcBuyPrice) : (data.mSrcSellPrice);
    }

    double InterRegionMarketDataModel::getDstPrice(const TypeData &data) const noexcept
    {
        return (mDstPriceType == PriceType::Buy) ? (data.mDstBuyPrice) : (data.mDstSellPrice);
    }
}
