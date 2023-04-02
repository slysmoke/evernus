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

#include <memory>
#include <vector>

#include "Character.h"
#include "Entity.h"
#include "Item.h"

namespace Evernus
{
    class AssetList
        : public Entity<uint>
    {
    public:
        typedef std::unique_ptr<Item> ItemType;
        typedef std::vector<ItemType> ItemList;
        typedef ItemList::iterator Iterator;
        typedef ItemList::const_iterator ConstIterator;

        using Entity::Entity;

        AssetList();
        AssetList(const AssetList &other);
        AssetList(AssetList &&other) = default;
        explicit AssetList(ItemList &&items);

        virtual ~AssetList();

        Character::IdType getCharacterId() const noexcept;
        void setCharacterId(Character::IdType id) noexcept;

        Iterator begin() noexcept;
        ConstIterator begin() const noexcept;
        Iterator end() noexcept;
        ConstIterator end() const noexcept;

        size_t size() const noexcept;

        void addItem(ItemType &&item);

        AssetList &operator =(const AssetList &other);
        AssetList &operator =(AssetList &&other) = default;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        ItemList mItems;
    };
}
