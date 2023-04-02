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
#include "TypeAggregatedMarketDataFilterProxyModel.h"

namespace Evernus
{
    TypeAggregatedMarketDataFilterProxyModel::TypeAggregatedMarketDataFilterProxyModel(int volumeColumn,
                                                                                       int marginColumn,
                                                                                       int buyPriceColumn,
                                                                                       int sellPriceColumn,
                                                                                       QObject *parent)
        : QSortFilterProxyModel{parent}
        , mVolumeColumn{volumeColumn}
        , mMarginColumn{marginColumn}
        , mBuyPriceColumn{buyPriceColumn}
        , mSellPriceColumn{sellPriceColumn}
    {
    }

    void TypeAggregatedMarketDataFilterProxyModel::setFilter(VolumeValueType minVolume,
                                                             VolumeValueType maxVolume,
                                                             MarginValueType minMargin,
                                                             MarginValueType maxMargin,
                                                             PriceValueType minBuyPrice,
                                                             PriceValueType maxBuyPrice,
                                                             PriceValueType minSellPrice,
                                                             PriceValueType maxSellPrice)
    {
        mMinVolume = std::move(minVolume);
        mMaxVolume = std::move(maxVolume);
        mMinMargin = std::move(minMargin);
        mMaxMargin = std::move(maxMargin);
        mMinBuyPrice = std::move(minBuyPrice);
        mMaxBuyPrice = std::move(maxBuyPrice);
        mMinSellPrice = std::move(minSellPrice);
        mMaxSellPrice = std::move(maxSellPrice);

        invalidateFilter();
    }

    bool TypeAggregatedMarketDataFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        const auto source = sourceModel();
        if (source != nullptr)
        {
            auto checkLimit = [source, sourceRow, &sourceParent](auto column, auto min, auto max) {
                if (min || max)
                {
                    using Type = std::remove_reference_t<decltype(*min)>;

                    const auto value
                        = source->data(source->index(sourceRow, column, sourceParent), Qt::UserRole).template value<Type>();
                    if (min && value < *min)
                        return false;
                    if (max && value > *max)
                        return false;
                }

                return true;
            };

            if (!checkLimit(mVolumeColumn, mMinVolume, mMaxVolume))
                return false;
            if (!checkLimit(mMarginColumn, mMinMargin, mMaxMargin))
                return false;
            if (!checkLimit(mBuyPriceColumn, mMinBuyPrice, mMaxBuyPrice))
                return false;
            if (!checkLimit(mSellPriceColumn, mMinSellPrice, mMaxSellPrice))
                return false;
        }

        return true;
    }
}
