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
#include "InterRegionMarketDataFilterProxyModel.h"

namespace Evernus
{
    InterRegionMarketDataFilterProxyModel::InterRegionMarketDataFilterProxyModel(int srcRegionColumn,
                                                                                 int dstRegionColumn,
                                                                                 int volumeColumn,
                                                                                 int marginColumn,
                                                                                 QObject *parent)
        : QSortFilterProxyModel{parent}
        , mSrcRegionColumn{srcRegionColumn}
        , mDstRegionColumn{dstRegionColumn}
        , mVolumeColumn{volumeColumn}
        , mMarginColumn{marginColumn}
    {
    }

    void InterRegionMarketDataFilterProxyModel::setFilter(RegionList srcRegions,
                                                          RegionList dstRegions,
                                                          VolumeValueType minVolume,
                                                          VolumeValueType maxVolume,
                                                          MarginValueType minMargin,
                                                          MarginValueType maxMargin)
    {
        mSrcRegions = std::move(srcRegions);
        mDstRegions = std::move(dstRegions);
        mMinVolume = std::move(minVolume);
        mMaxVolume = std::move(maxVolume);
        mMinMargin = std::move(minMargin);
        mMaxMargin = std::move(maxMargin);

        invalidateFilter();
    }

    bool InterRegionMarketDataFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        const auto source = sourceModel();
        if (Q_LIKELY(source != nullptr))
        {
            auto checkRegion = [=, &sourceParent](int column, const RegionList &regions) {
                const auto value
                    = source->data(source->index(sourceRow, column, sourceParent), Qt::UserRole + 1).toUInt();
                if (regions.find(value) == std::end(regions))
                    return false;

                return true;
            };

            if (!checkRegion(mSrcRegionColumn, mSrcRegions) || !checkRegion(mDstRegionColumn, mDstRegions))
                return false;

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
        }

        return true;
    }
}
