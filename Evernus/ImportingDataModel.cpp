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
#include <future>
#include <cmath>
#include <mutex>
#include <set>

#include <QCoreApplication>
#include <QSettings>
#include <QLocale>
#include <QColor>
#include <QIcon>

#include <QtConcurrent>

#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/scope_exit.hpp>

#include "MarketAnalysisSettings.h"
#include "EveDataProvider.h"
#include "PriceSettings.h"
#include "ExternalOrder.h"
#include "PriceUtils.h"
#include "TextUtils.h"
#include "MathUtils.h"

#include "ImportingDataModel.h"

using namespace boost::accumulators;

namespace Evernus
{
    ImportingDataModel::ImportingDataModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractTableModel{parent}
        , ModelWithTypes{}
        , mDataProvider{dataProvider}
    {
    }

    int ImportingDataModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant ImportingDataModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto column = index.column();
        const auto &data = mData[index.row()];

        switch (role) {
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (column) {
                case nameColumn:
                    return mDataProvider.getTypeName(data.mId);
                case avgVolumeColumn:
                    return locale.toString(data.mAvgVolume, 'f', 2);
                case medianDstVolume:
                    return locale.toString(data.mMedianVolume);
                case madDstVolume:
                    return locale.toString(data.mVolumeMAD, 'f', 2);
                case dstVolumeColumn:
                    return locale.toString(data.mDstVolume);
                case relativeDstVolumeColumn:
                    if (qFuzzyIsNull(data.mAvgVolume))
                        return QStringLiteral("0%1").arg(locale.percent());

                    return QStringLiteral("%1%2").arg(locale.toString(data.mDstVolume * 100 / data.mAvgVolume, 'f', 2)).arg(locale.percent());
                case srcOrderCountColumn:
                    return locale.toString(data.mSrcOrderCount);
                case dstOrderCountColumn:
                    return locale.toString(data.mDstOrderCount);
                case dstPriceColumn:
                    return TextUtils::currencyToString(data.mDstPrice, locale);
                case srcPriceColumn:
                    return TextUtils::currencyToString(data.mSrcPrice, locale);
                case importPriceColumn:
                    return TextUtils::currencyToString(data.mImportPrice, locale);
                case priceDifferenceColumn:
                    return TextUtils::currencyToString(data.mPriceDifference, locale);
                case marginColumn:
                    return QStringLiteral("%1%2").arg(locale.toString(data.mMargin, 'f', 2)).arg(locale.percent());
                case projectedProfitColumn:
                    return TextUtils::currencyToString(data.mProjectedProfit, locale);
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mId);
            case avgVolumeColumn:
                return data.mAvgVolume;
            case medianDstVolume:
                return data.mMedianVolume;
            case madDstVolume:
                return data.mVolumeMAD;
            case dstVolumeColumn:
                return data.mDstVolume;
            case relativeDstVolumeColumn:
                if (qFuzzyIsNull(data.mAvgVolume))
                    return 0;

                return data.mDstVolume * 100 / data.mAvgVolume;
            case srcOrderCountColumn:
                return data.mSrcOrderCount;
            case dstOrderCountColumn:
                return data.mDstOrderCount;
            case dstPriceColumn:
                return data.mDstPrice;
            case srcPriceColumn:
                return data.mSrcPrice;
            case importPriceColumn:
                return data.mImportPrice;
            case priceDifferenceColumn:
                return data.mPriceDifference;
            case marginColumn:
                return data.mMargin;
            case projectedProfitColumn:
                return data.mProjectedProfit;
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

        return {};
    }

    QVariant ImportingDataModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case avgVolumeColumn:
                return tr("Avg. dst. volume");
            case medianDstVolume:
                return tr("Median dst. volume");
            case madDstVolume:
                return tr("Dst. volume mean absolute deviation");
            case dstVolumeColumn:
                return tr("Dst. remaining volume");
            case relativeDstVolumeColumn:
                return tr("Relative dst. remaining volume");
            case srcOrderCountColumn:
                return tr("Source order count");
            case dstOrderCountColumn:
                return tr("Destination order count");
            case dstPriceColumn:
                return tr("5% volume destination price");
            case srcPriceColumn:
                return tr("5% volume source price");
            case importPriceColumn:
                return tr("Import price (src. price + price per m³ + collateral)");
            case priceDifferenceColumn:
                return tr("Price difference");
            case marginColumn:
                return tr("Margin");
            case projectedProfitColumn:
                return tr("Projected profit");
            }
        }

        return QVariant{};
    }

    int ImportingDataModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void ImportingDataModel::setCharacter(std::shared_ptr<Character> character)
    {
        beginResetModel();
        mCharacter = std::move(character);
        mData.clear();
        endResetModel();
    }

    void ImportingDataModel::discardBogusOrders(bool flag) noexcept
    {
        mDiscardBogusOrders = flag;
    }

    void ImportingDataModel::setBogusOrderThreshold(double value) noexcept
    {
        mBogusOrderThreshold = value;
    }

    EveType::IdType ImportingDataModel::getTypeId(const QModelIndex &index) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return EveType::invalidId;

        return mData[index.row()].mId;
    }

    void ImportingDataModel::setOrderData(const std::vector<ExternalOrder> &orders,
                                          const HistoryRegionMap &history,
                                          quint64 srcStation,
                                          quint64 dstStation,
                                          PriceType srcPriceType,
                                          PriceType dstPriceType,
                                          int analysisDays,
                                          int aggrDays,
                                          double pricePerM3,
                                          double collateral,
                                          PriceType collateralType,
                                          bool hideEmptySell)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        const auto dstHistory = history.find(mDataProvider.getStationRegionId(dstStation));
        if (dstHistory == std::end(history))
            return;

        const auto srcHistory = history.find(mDataProvider.getStationRegionId(srcStation));
        if (srcHistory == std::end(history))
            return;

        struct TypeMapData
        {
            quint64 mTotalVolume = 0;
            quint64 mMedianVolume = 0;
            double mDstPrice = 0.;
            double mSrcPrice = 0.;
            quint64 mSrcOrderCount = 0;
            quint64 mDstOrderCount = 0;
        };

        TypeMap<TypeMapData> typeMap;
        TypeMap<quint64> srcVolumes, dstVolumes, dstSellVolumes;
        TypeMap<std::multiset<std::reference_wrapper<const ExternalOrder>, ExternalOrder::LowToHigh>> dstOrders;
        TypeMap<std::multiset<std::reference_wrapper<const ExternalOrder>, ExternalOrder::HighToLow>> srcOrders;

        // gather prices and volumes from dst orders - we need those to calculate percentile dst price
        auto dstFuture = std::async(std::launch::async, [&] {
            const auto dstOrderFilter = [=](const auto &order) {
                return order.getStationId() == dstStation;
            };

            for (const auto &order : orders | boost::adaptors::filtered(dstOrderFilter))
            {
                const auto typeId = order.getTypeId();
                const auto type = order.getType();

                if (dstPriceType == type)
                {
                    dstOrders[typeId].emplace(std::cref(order));
                    dstVolumes[typeId] += order.getVolumeRemaining();
                }

                if (type == ExternalOrder::Type::Sell)
                    dstSellVolumes[typeId] += order.getVolumeRemaining();
            }
        });

        // gather prices and volumes from src orders - we need those to calculate percentile src price
        auto srcFuture = std::async(std::launch::async, [&] {
            const auto srcOrderFilter = [=](const auto &order) {
                return order.getStationId() == srcStation && srcPriceType == order.getType();
            };

            for (const auto &order : orders | boost::adaptors::filtered(srcOrderFilter))
            {
                const auto typeId = order.getTypeId();
                srcOrders[typeId].emplace(std::cref(order));
                srcVolumes[typeId] += order.getVolumeRemaining();
            }
        });

        const auto historyLimit = QDate::currentDate().addDays(-analysisDays + 1);

        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

        dstFuture.get();
        srcFuture.get();

        QSettings settings;

        const auto volumePercentile = 0.05;
        const auto preferredMargin
            = settings.value(PriceSettings::preferredMarginKey, PriceSettings::preferredMarginDefault).toDouble() / 100.;

        // fill our type map with order data
        for (const auto &order : orders)
        {
            const auto typeId = order.getTypeId();
            const auto type = typeMap.find(typeId);
            if (type != std::end(typeMap))
                continue;

            auto &data = typeMap[typeId];

            accumulator_set<double, stats<tag::mean>> dstPriceAcc;
            accumulator_set<double, stats<tag::mean>> srcPriceAcc;

            std::vector<quint64> historyVolumes(analysisDays);
            auto curHistoryVolume = std::begin(historyVolumes);

            // go through dst history to calculate avg price and total trade volume
            const auto dstTypeHistory = dstHistory->second.find(typeId);
            if (Q_LIKELY(dstTypeHistory != std::end(dstHistory->second)))
            {
                for (const auto &timePoint : boost::adaptors::reverse(dstTypeHistory->second))
                {
                    if (Q_UNLIKELY(timePoint.first < historyLimit))
                        break;

                    data.mTotalVolume += timePoint.second.mVolume;
                    dstPriceAcc(timePoint.second.mAvgPrice);

                    *(curHistoryVolume++) = timePoint.second.mVolume;
                }
            }

            std::nth_element(std::begin(historyVolumes), std::begin(historyVolumes) + historyVolumes.size() / 2, std::end(historyVolumes));
            data.mMedianVolume = historyVolumes[historyVolumes.size() / 2];

            const auto srcTypeHistory = srcHistory->second.find(typeId);
            if (Q_LIKELY(srcTypeHistory != std::end(srcHistory->second)))
            {
                for (const auto &timePoint : boost::adaptors::reverse(srcTypeHistory->second))
                {
                    if (Q_UNLIKELY(timePoint.first < historyLimit))
                        break;

                    srcPriceAcc(timePoint.second.mAvgPrice);
                }
            }

            const auto &typeSrcOrders = srcOrders[typeId];
            const auto &typeDstOrders = dstOrders[typeId];

            data.mSrcOrderCount = typeSrcOrders.size();
            data.mDstOrderCount = typeDstOrders.size();

            // dst orders are by default sorted from lowest price, which means we need to reverse them if we
            // want to sell to buy orders, which are highest first
            if (dstPriceType == PriceType::Sell)
            {
                data.mDstPrice = MathUtils::calcPercentile(typeDstOrders,
                                                           dstVolumes[typeId] * volumePercentile,
                                                           mean(dstPriceAcc),
                                                           mDiscardBogusOrders,
                                                           mBogusOrderThreshold);
            }
            else
            {
                data.mDstPrice = MathUtils::calcPercentile(boost::adaptors::reverse(typeDstOrders),
                                                           dstVolumes[typeId] * volumePercentile,
                                                           mean(dstPriceAcc),
                                                           mDiscardBogusOrders,
                                                           mBogusOrderThreshold);
            }

            // same logic for src orders
            if (srcPriceType == PriceType::Buy)
            {
                data.mSrcPrice = MathUtils::calcPercentile(typeSrcOrders,
                                                           srcVolumes[typeId] * volumePercentile,
                                                           mean(srcPriceAcc),
                                                           mDiscardBogusOrders,
                                                           mBogusOrderThreshold);
            }
            else
            {
                data.mSrcPrice = MathUtils::calcPercentile(boost::adaptors::reverse(typeSrcOrders),
                                                           srcVolumes[typeId] * volumePercentile,
                                                           mean(srcPriceAcc),
                                                           mDiscardBogusOrders,
                                                           mBogusOrderThreshold);
            }

            // check if this was traded at all
            if (qFuzzyIsNull(data.mDstPrice))
                data.mDstPrice = data.mSrcPrice * (1 + preferredMargin);

            QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
        }

        PriceUtils::Taxes taxes;

        const auto useSkillsForDifference = mCharacter && settings.value(
            MarketAnalysisSettings::useSkillsForDifferenceKey, MarketAnalysisSettings::useSkillsForDifferenceDefault).toBool();

        if (useSkillsForDifference)
            taxes = PriceUtils::calculateTaxes(*mCharacter);

        mData.reserve(typeMap.size());

        hideEmptySell = hideEmptySell && srcPriceType == PriceType::Sell;

        std::mutex dataMutex;

        QtConcurrent::blockingMap(typeMap, [&](const auto &type) {
            if (hideEmptySell && type.second.mSrcOrderCount == 0)
                return;

            const auto dstVolume = dstSellVolumes.find(type.first);

            TypeData data;
            data.mId = type.first;
            data.mAvgVolume = static_cast<double>(type.second.mTotalVolume) * aggrDays / analysisDays;
            data.mMedianVolume = type.second.mMedianVolume;
            data.mDstVolume = (dstVolume == std::end(dstSellVolumes)) ? (0) : (dstVolume->second);
            data.mSrcOrderCount = type.second.mSrcOrderCount;
            data.mDstOrderCount = type.second.mDstOrderCount;

            auto absDeviationSum = 0.;

            const auto dstTypeHistory = dstHistory->second.find(data.mId);
            if (Q_LIKELY(dstTypeHistory != std::end(dstHistory->second)))
            {
                for (const auto &timePoint : boost::adaptors::reverse(dstTypeHistory->second))
                {
                    if (Q_UNLIKELY(timePoint.first < historyLimit))
                        break;

                    absDeviationSum += std::abs(timePoint.second.mVolume - data.mAvgVolume);
                }
            }

            data.mVolumeMAD = absDeviationSum / analysisDays;

            if (useSkillsForDifference)
            {
                data.mDstPrice = (dstPriceType == PriceType::Sell) ?
                                 (PriceUtils::getSellPrice(type.second.mDstPrice, taxes)) :
                                 (PriceUtils::getSellPrice(type.second.mDstPrice, taxes, false));
                data.mSrcPrice = (srcPriceType == PriceType::Buy) ?
                                 (PriceUtils::getBuyPrice(type.second.mSrcPrice, taxes)) :
                                 (PriceUtils::getBuyPrice(type.second.mSrcPrice, taxes, false));
            }
            else
            {
                data.mDstPrice = type.second.mDstPrice;
                data.mSrcPrice = type.second.mSrcPrice;
            }

            const auto collateralPrice = (collateralType == PriceType::Buy) ? (data.mSrcPrice) : (data.mDstPrice);

            data.mImportPrice = data.mSrcPrice + collateralPrice * collateral + mDataProvider.getTypeVolume(data.mId) * pricePerM3;
            data.mPriceDifference = data.mDstPrice - data.mImportPrice;
            data.mMargin = (qFuzzyIsNull(data.mDstPrice)) ? (0.) : (100. * data.mPriceDifference / data.mDstPrice);
            data.mProjectedProfit = data.mAvgVolume * data.mPriceDifference;

            std::lock_guard<std::mutex> lock{dataMutex};
            mData.emplace_back(std::move(data));
        });
    }

    void ImportingDataModel::reset()
    {
        beginResetModel();
        mData.clear();
        endResetModel();
    }
}
