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
#include "RegionStationPreset.h"

namespace Evernus
{
    RegionStationPreset::StationId RegionStationPreset::getSrcStationId() const
    {
        return mSrcStationId;
    }

    void RegionStationPreset::setSrcStationId(StationId station)
    {
        mSrcStationId = std::move(station);
    }

    RegionStationPreset::StationId RegionStationPreset::getDstStationId() const
    {
        return mDstStationId;
    }

    void RegionStationPreset::setDstStationId(StationId station)
    {
        mDstStationId = std::move(station);
    }

    RegionStationPreset::RegionSet RegionStationPreset::getSrcRegions() const &
    {
        return mSrcRegions;
    }

    RegionStationPreset::RegionSet &&RegionStationPreset::getSrcRegions() && noexcept
    {
        return std::move(mSrcRegions);
    }

    void RegionStationPreset::setSrcRegions(const RegionSet &regions)
    {
        mSrcRegions = regions;
    }

    void RegionStationPreset::setSrcRegions(RegionSet &&regions) noexcept
    {
        mSrcRegions = std::move(regions);
    }

    RegionStationPreset::RegionSet RegionStationPreset::getDstRegions() const &
    {
        return mDstRegions;
    }

    RegionStationPreset::RegionSet &&RegionStationPreset::getDstRegions() && noexcept
    {
        return std::move(mDstRegions);
    }

    void RegionStationPreset::setDstRegions(const RegionSet &regions)
    {
        mDstRegions = regions;
    }

    void RegionStationPreset::setDstRegions(RegionSet &&regions) noexcept
    {
        mDstRegions = std::move(regions);
    }
}
