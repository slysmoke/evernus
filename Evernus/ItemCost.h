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

#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class ItemCost
        : public Entity<quint64>
    {
    public:
        using Entity::Entity;

        ItemCost() = default;
        ItemCost(const ItemCost &) = default;
        ItemCost(ItemCost &&) = default;
        virtual ~ItemCost() = default;

        Character::IdType getCharacterId() const;
        void setCharacterId(Character::IdType id);

        EveType::IdType getTypeId() const;
        void setTypeId(EveType::IdType id);

        double getAdjustedCost() const noexcept;
        double getCost() const noexcept;
        void setCost(double cost) noexcept;

        ItemCost &operator =(const ItemCost &) = default;
        ItemCost &operator =(ItemCost &&) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        EveType::IdType mTypeId = EveType::invalidId;
        double mCost = 0.;
    };
}
