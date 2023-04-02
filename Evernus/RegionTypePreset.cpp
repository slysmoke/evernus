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
#include "RegionTypePreset.h"

namespace Evernus
{
    RegionTypePreset::TypeSet RegionTypePreset::getTypes() const &
    {
        return mTypes;
    }

    RegionTypePreset::TypeSet &&RegionTypePreset::getTypes() && noexcept
    {
        return std::move(mTypes);
    }

    void RegionTypePreset::setTypes(const TypeSet &types)
    {
        mTypes = types;
    }

    void RegionTypePreset::setTypes(TypeSet &&types) noexcept
    {
        mTypes = std::move(types);
    }

    RegionTypePreset::RegionSet RegionTypePreset::getRegions() const &
    {
        return mRegions;
    }

    RegionTypePreset::RegionSet &&RegionTypePreset::getRegions() && noexcept
    {
        return std::move(mRegions);
    }

    void RegionTypePreset::setRegions(const RegionSet &regions)
    {
        mRegions = regions;
    }

    void RegionTypePreset::setRegions(RegionSet &&regions) noexcept
    {
        mRegions = std::move(regions);
    }
}
