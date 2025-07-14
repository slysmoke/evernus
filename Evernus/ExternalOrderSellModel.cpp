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
#include <unordered_map>
#include <algorithm>

#include <QLocale>
#include <QColor>

#include "ExternalOrderRepository.h"
#include "CharacterRepository.h"
#include "MarketOrderProvider.h"
#include "ItemCostProvider.h"
#include "EveDataProvider.h"
#include "MathUtils.h"
#include "TextUtils.h"
#include "DarkModeColor.h"

#include "ExternalOrderSellModel.h"

namespace Evernus
{
    ExternalOrderSellModel::ExternalOrderSellModel(const EveDataProvider &dataProvider,
                                                   const ExternalOrderRepository &orderRepo,
                                                   const CharacterRepository &characterRepo,
                                                   const MarketOrderProvider &orderProvider,
                                                   const MarketOrderProvider &corpOrderProvider,
                                                   const ItemCostProvider &costProvider,
                                                   QObject *parent)
        : ExternalOrderModel{dataProvider, parent}
        , mOrderRepo{orderRepo}
        , mCharacterRepo{characterRepo}
        , mOrderProvider{orderProvider}
        , mCorpOrderProvider{corpOrderProvider}
        , mCostProvider{costProvider}
    {
    }
   
    int ExternalOrderSellModel::columnCount(const QModelIndex &parent) const
    {
        return (mGrouping == Grouping::None) ? (static_cast<int>(numUngroupedColumns)) : (static_cast<int>(numGroupedColumns));
    }

    QVariant ExternalOrderSellModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            if (mGrouping == Grouping::None)
            {
                switch (section) {
                case stationColumn:
                    return tr("Station");
                case deviationColumn:
                    return tr("Deviation");
                case priceColumn:
                    return tr("Price");
                case volumeColumn:
                    return tr("Volume");
                case totalProfitColumn:
                    return tr("Total profit");
                case totalSizeColumn:
                    return tr("Total size");
                case issuedColumn:
                    return tr("Issued");
                case durationColumn:
                    return tr("Time left");
                case updatedColumn:
                    return tr("Last modified");
                case regionColumn:
                    return tr("Region");
                }
            }
            else
            {
                switch (section) {
                case groupByColumn:
                    switch (mGrouping) {
                    case Grouping::Station:
                        return tr("Station");
                    case Grouping::System:
                        return tr("Solar system");
                    case Grouping::Region:
                        return tr("Region");
                    default:
                        break;
                    }
                    break;
                case lowestPriceColumn:
                    return tr("Lowest price");
                case medianPriceColumn:
                    return tr("Median price");
                case highestPriceColumn:
                    return tr("Highest price");
                case volumeColumn:
                    return tr("Volume");
                case groupedTotalProfitColumn:
                    return tr("Total profit");
                case ordersColumn:
                    return tr("Orders");
                case groupedTotalSizeColumn:
                    return tr("Total size");
                }
            }
        }

        return QVariant{};
    }

    int ExternalOrderSellModel::getPriceColumn() const
    {
        return (mGrouping == Grouping::None) ? (static_cast<int>(priceColumn)) : (static_cast<int>(lowestPriceColumn));
    }

    Qt::SortOrder ExternalOrderSellModel::getPriceSortOrder() const
    {
        return Qt::AscendingOrder;
    }

    int ExternalOrderSellModel::getVolumeColumn() const
    {
        return volumeColumn;
    }

    void ExternalOrderSellModel::setCharacter(Character::IdType id)
    {
        beginResetModel();

        mCharacterId = id;
        mOwnOrders.clear();

        const auto orders = mOrderProvider.getSellOrders(mCharacterId);
        for (const auto &order : orders)
            mOwnOrders.emplace(order->getId());

        try
        {
            const auto corpOrders = mCorpOrderProvider.getSellOrdersForCorporation(mCharacterRepo.getCorporationId(mCharacterId));
            for (const auto &order : corpOrders)
                mOwnOrders.emplace(order->getId());
        }
        catch (const CharacterRepository::NotFoundException &)
        {
        }

        endResetModel();
    }

    void ExternalOrderSellModel::reset()
    {
        beginResetModel();

        if (mStationId != 0)
            mOrders = mOrderRepo.fetchSellByTypeAndStation(mTypeId, mStationId);
        else if (mSolarSystemId != 0)
            mOrders = mOrderRepo.fetchSellByTypeAndSolarSystem(mTypeId, mSolarSystemId);
        else if (mRegionId != 0)
            mOrders = mOrderRepo.fetchSellByTypeAndRegion(mTypeId, mRegionId);
        else
            mOrders = mOrderRepo.fetchSellByType(mTypeId);

        const auto aggrData = MathUtils::calcAggregates(mOrders, mDataProvider);

        mTotalPrice = aggrData.mTotalPrice;
        mMinPrice = aggrData.mMinPrice;
        mMaxPrice = aggrData.mMaxPrice;
        mMedianPrice = aggrData.mMedianPrice;
        mTotalVolume = aggrData.mTotalVolume;
        mTotalSize = aggrData.mTotalSize;

        if (mGrouping != Grouping::None)
            refreshGroupedData();

        endResetModel();
    }

    double ExternalOrderSellModel::getPrice(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0.;

        switch (mGrouping) {
        case Grouping::None:
            return mOrders[index.row()]->getPrice();
        case Grouping::Station:
        case Grouping::System:
        case Grouping::Region:
            return mGroupedData[index.row()].mLowestPrice;
        }

        return 0.;
    }

    double ExternalOrderSellModel::computeDeviation(const ExternalOrder &order) const
    {
        switch (mDeviationType) {
        case DeviationSourceType::Median:
            return (mMedianPrice == 0.) ? (0.) : ((order.getPrice() - mMedianPrice) / mMedianPrice);
        case DeviationSourceType::Best:
            {
                const auto value = mDataProvider.getTypeBuyPrice(order.getTypeId(), order.getStationId())->getPrice();
                return (value == 0.) ? (0.) : ((order.getPrice() - value) / value);
            }
        case DeviationSourceType::Cost:
            {
                const auto cost = mCostProvider.fetchForCharacterAndType(mCharacterId, order.getTypeId())->getAdjustedCost();
                return (cost == 0.) ? (0.) : ((order.getPrice() - cost) / cost);
            }
        case DeviationSourceType::Fixed:
            return (mDeviationValue == 0.) ? (0.) : ((order.getPrice() - mDeviationValue) / mDeviationValue);
        default:
            return 0.;
        }
    }

    void ExternalOrderSellModel::refreshGroupedData()
    {
        switch (mGrouping) {
        case Grouping::Station:
            fillGroupedData<quint64, &ExternalOrder::getStationId>();
            break;
        case Grouping::System:
            fillGroupedData<uint, &ExternalOrder::getSolarSystemId>();
            break;
        case Grouping::Region:
            fillGroupedData<uint, &ExternalOrder::getRegionId>();
            break;
        default:
            break;
        }
    }

    QVariant ExternalOrderSellModel::getUngroupedData(int column, int role, const ExternalOrder &order) const
    {
        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case stationColumn:
                    return mDataProvider.getLocationName(order.getStationId());
                case deviationColumn:
                    return QString{"%1%2"}.arg(static_cast<int>(computeDeviation(order) * 100.)).arg(locale.percent());
                case priceColumn:
                    return TextUtils::currencyToString(order.getPrice(), locale);
                case volumeColumn:
                    return QString{"%1/%2"}.arg(locale.toString(order.getVolumeRemaining())).arg(locale.toString(order.getVolumeEntered()));
                case totalProfitColumn:
                    return TextUtils::currencyToString(order.getVolumeRemaining() * order.getPrice(),locale);
                case totalSizeColumn:
                    return QString{"%1m³"}.arg(locale.toString(order.getVolumeRemaining() * mDataProvider.getTypeVolume(order.getTypeId()), 'f', 2));
                case issuedColumn:
                    return TextUtils::dateTimeToString(order.getIssued().toLocalTime(), locale);
                case durationColumn:
                    {
                        const auto timeEnd = order.getIssued().addDays(order.getDuration()).toMSecsSinceEpoch() / 1000;
                        const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                        if (timeEnd > timeCur)
                            return TextUtils::secondsToString(std::chrono::seconds{timeEnd - timeCur});
                    }
                    break;
                case updatedColumn:
                {
                    const auto timeStart = order.getIssued().toMSecsSinceEpoch() / 1000;
                    const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                    if (timeCur > timeStart)
                        return TextUtils::secondsToString(std::chrono::seconds{ timeCur - timeStart });
                }
                    
                case regionColumn:
                    return mDataProvider.getRegionName(order.getRegionId());
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case stationColumn:
                return mDataProvider.getLocationName(order.getStationId());
            case deviationColumn:
                return computeDeviation(order);
            case priceColumn:
                return order.getPrice();
            case volumeColumn:
                return QVariantList{} << order.getVolumeRemaining() << order.getVolumeEntered();
            case totalProfitColumn:
                return order.getVolumeRemaining() * order.getPrice();
            case totalSizeColumn:
                return order.getVolumeRemaining() * mDataProvider.getTypeVolume(order.getTypeId());
            case issuedColumn:
                return order.getIssued();
            case durationColumn:
                {
                    const auto timeEnd = order.getIssued().addDays(order.getDuration()).toMSecsSinceEpoch() / 1000;
                    const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                    if (timeEnd > timeCur)
                        return timeEnd - timeCur;
                }
                break;
            case updatedColumn:
            {
                const auto timeStart = order.getIssued().toMSecsSinceEpoch() / 1000;
                const auto timeCur = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000;

                if (timeCur > timeStart)
                    return  timeCur - timeStart;
            }
            case regionColumn:
                return mDataProvider.getRegionName(order.getRegionId());
            }
            break;
        case Qt::ForegroundRole:
            if (column == totalProfitColumn)
            {
                if (mPriceColorMode == PriceColorMode::Direction)

                    return Evernus::DarkModeColor::green();
                    //return QColor{Qt::green};

                return (computeDeviation(order) < 0.) ? (QColor{Qt::red}) : (Evernus::DarkModeColor::green());
            }
            break;
        case Qt::BackgroundRole:
            if (mOwnOrders.find(order.getId()) != std::end(mOwnOrders))

                //return QColor{ 43, 61, 255 };
                return Evernus::DarkModeColor::orderHighlight();
            break;
        case Qt::ToolTipRole:
            if (mOwnOrders.find(order.getId()) != std::end(mOwnOrders))
                return tr("Your order");
            break;
        case Qt::TextAlignmentRole:
            if (column == volumeColumn || column == deviationColumn)
                return Qt::AlignRight;
        }

        return QVariant{};
    }

    QVariant ExternalOrderSellModel::getGenericGroupedData(int column, int role, const GroupedData &data) const
    {
        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case lowestPriceColumn:
                    return TextUtils::currencyToString(data.mLowestPrice, locale);
                case medianPriceColumn:
                    return TextUtils::currencyToString(data.mMedianPrice, locale);
                case highestPriceColumn:
                    return TextUtils::currencyToString(data.mHighestPrice, locale);
                case volumeColumn:
                    return QString{"%1/%2"}.arg(locale.toString(data.mVolumeRemaining)).arg(locale.toString(data.mVolumeEntered));
                case groupedTotalProfitColumn:
                    return TextUtils::currencyToString(data.mTotalProfit, locale);
                case ordersColumn:
                    return locale.toString(data.mCount);
                case groupedTotalSizeColumn:
                    return QString{"%1m³"}.arg(locale.toString(data.mVolumeRemaining * mDataProvider.getTypeVolume(mTypeId), 'f', 2));
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case lowestPriceColumn:
                return data.mLowestPrice;
            case medianPriceColumn:
                return data.mMedianPrice;
            case highestPriceColumn:
                return data.mHighestPrice;
            case volumeColumn:
                return QVariantList{} << data.mVolumeRemaining << data.mVolumeEntered;
            case groupedTotalProfitColumn:
                return data.mTotalProfit;
            case ordersColumn:
                return data.mCount;
            case groupedTotalSizeColumn:
                return data.mVolumeRemaining * mDataProvider.getTypeVolume(mTypeId);
            }
            break;
        case Qt::ForegroundRole:
            if (column == groupedTotalProfitColumn)
                return Evernus::DarkModeColor::green();
            break;
        case Qt::TextAlignmentRole:
            if (column == volumeColumn)
                return Qt::AlignRight;
        }

        return QVariant{};
    }

    template<class Id, Id (ExternalOrder::* Func)() const>
    void ExternalOrderSellModel::fillGroupedData()
    {
        mGroupedData.clear();

        std::unordered_map<quint64, GroupedData> data;
        std::unordered_map<quint64, std::vector<double>> prices;

        for (const auto &order : mOrders)
        {
            const auto id = (order.get()->*Func)();
            auto &curData = data[id];
            curData.mId = id;

            const auto price = order->getPrice();
            if (curData.mLowestPrice > price)
                curData.mLowestPrice = price;
            if (curData.mHighestPrice < price)
                curData.mHighestPrice = price;

            prices[id].emplace_back(price);

            curData.mVolumeEntered += order->getVolumeEntered();
            curData.mVolumeRemaining += order->getVolumeRemaining();
            curData.mTotalProfit += order->getVolumeRemaining() * price;

            ++curData.mCount;

            curData.mTotalSize += order->getVolumeRemaining() * mDataProvider.getTypeVolume(order->getTypeId());
        }

        mGroupedData.reserve(data.size());

        for (auto &curData : data)
        {
            mGroupedData.emplace_back(std::move(curData.second));

            auto &curPrices = prices[curData.first];
            std::nth_element(std::begin(curPrices), std::next(std::begin(curPrices), curPrices.size() / 2), std::end(curPrices));

            mGroupedData.back().mMedianPrice = curPrices[curPrices.size() / 2];
        }
    }
}
