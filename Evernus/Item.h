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

#include <optional>

#include "ItemData.h"
#include "Entity.h"

namespace Evernus
{
    class Item
        : public Entity<ItemData::IdType>
    {
    public:
        using ItemList = std::vector<std::unique_ptr<Item>>;
        using ItemIterator = ItemList::iterator;
        using ConstItemIterator = ItemList::const_iterator;

        using ParentIdType = std::optional<ItemData::IdType>;
        using CustomValueType = std::optional<double>;
        using BPCType = std::optional<bool>;

        using Entity::Entity;

        Item() = default;
        Item(const Item &other);
        Item(Item &&) = default;
        virtual ~Item() = default;

        ParentIdType getParentId() const noexcept;
        void setParentId(const ParentIdType &id) noexcept;

        uint getListId() const noexcept;
        void setListId(uint id) noexcept;

        ItemData::TypeIdType getTypeId() const;
        void setTypeId(const ItemData::TypeIdType &id);

        ItemData::LocationIdType getLocationId() const;
        void setLocationId(const ItemData::LocationIdType &id);

        uint getQuantity() const noexcept;
        void setQuantity(uint value) noexcept;

        int getRawQuantity() const noexcept;
        void setRawQuantity(int value) noexcept;

        ItemData getItemData() const &;
        ItemData &&getItemData() && noexcept;
        void setItemData(const ItemData &data);
        void setItemData(ItemData &&data);

        ItemIterator begin() noexcept;
        ConstItemIterator begin() const noexcept;
        ItemIterator end() noexcept;
        ConstItemIterator end() const noexcept;

        CustomValueType getCustomValue() const;
        void setCustomValue(CustomValueType value);

        BPCType getBPCFlag() const noexcept;
        void setBPCFlag(BPCType value) noexcept;

        size_t getChildCount() const noexcept;

        void addItem(std::unique_ptr<Item> &&item);

        bool isBPC() const noexcept;

        Item &operator =(const Item &other);
        Item &operator =(Item &&) = default;

    private:
        ParentIdType mParentId;
        uint mListId = 0;
        ItemData mData;
        ItemList mContents;
        CustomValueType mCustomValue;
        BPCType mIsBPC;
    };
}
