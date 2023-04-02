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

#include <QString>

#include "EveType.h"

namespace Evernus
{
    class RegionTypePreset final
        : public Entity<QString>
    {
    public:
        using TypeSet = std::unordered_set<EveType::IdType>;
        using RegionSet = std::unordered_set<uint>;

        using Entity::Entity;
        RegionTypePreset() = default;
        RegionTypePreset(const RegionTypePreset &) = default;
        RegionTypePreset(RegionTypePreset &&) = default;
        virtual ~RegionTypePreset() = default;

        TypeSet getTypes() const &;
        TypeSet &&getTypes() && noexcept;
        void setTypes(const TypeSet &types);
        void setTypes(TypeSet &&types) noexcept;

        RegionSet getRegions() const &;
        RegionSet &&getRegions() && noexcept;
        void setRegions(const RegionSet &regions);
        void setRegions(RegionSet &&regions) noexcept;

        RegionTypePreset &operator =(const RegionTypePreset &) = default;
        RegionTypePreset &operator =(RegionTypePreset &&) = default;

    private:
        TypeSet mTypes;
        RegionSet mRegions;
    };
}
