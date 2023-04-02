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

#include <unordered_set>

#include <optional>

#include <QSortFilterProxyModel>

namespace Evernus
{
    class InterRegionMarketDataFilterProxyModel
        : public QSortFilterProxyModel
    {
    public:
        using VolumeValueType = std::optional<uint>;
        using MarginValueType = std::optional<double>;

        using RegionList = std::unordered_set<uint>;

        InterRegionMarketDataFilterProxyModel(int srcRegionColumn,
                                              int dstRegionColumn,
                                              int volumeColumn,
                                              int marginColumn,
                                              QObject *parent = nullptr);
        virtual ~InterRegionMarketDataFilterProxyModel() = default;

        void setFilter(RegionList srcRegions,
                       RegionList dstRegions,
                       VolumeValueType minVolume,
                       VolumeValueType maxVolume,
                       MarginValueType minMargin,
                       MarginValueType maxMargin);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        int mSrcRegionColumn = 0;
        int mDstRegionColumn = 0;
        int mVolumeColumn = 0;
        int mMarginColumn = 0;

        RegionList mSrcRegions;
        RegionList mDstRegions;

        VolumeValueType mMinVolume, mMaxVolume;
        MarginValueType mMinMargin, mMaxMargin;
    };
}
