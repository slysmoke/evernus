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

#include <vector>
#include <memory>

#include "ItemCost.h"

namespace Evernus
{
    class ItemCostProvider
    {
    public:
        typedef std::vector<std::shared_ptr<ItemCost>> CostList;

        struct NotFoundException : std::exception { };

        ItemCostProvider() = default;
        virtual ~ItemCostProvider() = default;

        virtual std::shared_ptr<ItemCost> fetchForCharacterAndType(Character::IdType characterId, EveType::IdType typeId) const = 0;
        virtual CostList fetchForCharacter(Character::IdType characterId) const = 0;
        virtual void setForCharacterAndType(Character::IdType characterId, EveType::IdType typeId, double value) = 0;

        virtual std::shared_ptr<ItemCost> findItemCost(ItemCost::IdType id) const = 0;
        virtual void removeItemCost(ItemCost::IdType id) const = 0;
        virtual void storeItemCost(ItemCost &cost) const = 0;
        virtual void removeAllItemCosts(Character::IdType characterId) const = 0;
    };
}
