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
#pragma once

#include <optional>

#include <QSortFilterProxyModel>

namespace Evernus
{
    class TypeAggregatedMarketDataFilterProxyModel
        : public QSortFilterProxyModel
    {
    public:
        using VolumeValueType = std::optional<uint>;
        using MarginValueType = std::optional<double>;
        using PriceValueType = std::optional<double>;

        TypeAggregatedMarketDataFilterProxyModel(int volumeColumn,
                                                 int marginColumn,
                                                 int buyPriceColumn,
                                                 int sellPriceColumn,
                                                 QObject *parent = nullptr);
        virtual ~TypeAggregatedMarketDataFilterProxyModel() = default;

        void setFilter(VolumeValueType minVolume,
                       VolumeValueType maxVolume,
                       MarginValueType minMargin,
                       MarginValueType maxMargin,
                       PriceValueType minBuyPrice,
                       PriceValueType maxBuyPrice,
                       PriceValueType minSellPrice,
                       PriceValueType maxSellPrice);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        int mVolumeColumn = 0;
        int mMarginColumn = 0;
        int mBuyPriceColumn = 0;
        int mSellPriceColumn = 0;

        VolumeValueType mMinVolume, mMaxVolume;
        MarginValueType mMinMargin, mMaxMargin;
        PriceValueType mMinBuyPrice, mMaxBuyPrice;
        PriceValueType mMinSellPrice, mMaxSellPrice;
    };
}
