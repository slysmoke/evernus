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
#include "Item.h"

namespace Evernus
{
    Item::Item(const Item &other)
        : Entity{other}
        , mParentId{other.mParentId}
        , mListId{other.mListId}
        , mData{other.mData}
        , mCustomValue{other.mCustomValue}
        , mIsBPC{other.mIsBPC}
    {
        for (const auto &item : other)
            addItem(std::make_unique<Item>(*item));
    }

    Item::ParentIdType Item::getParentId() const noexcept
    {
        return mParentId;
    }

    void Item::setParentId(const ParentIdType &id) noexcept
    {
        mParentId = id;
    }

    uint Item::getListId() const noexcept
    {
        return mListId;
    }

    void Item::setListId(uint id) noexcept
    {
        mListId = id;

        for (auto &item : mContents)
            item->setListId(mListId);
    }

    ItemData::TypeIdType Item::getTypeId() const
    {
        return mData.mTypeId;
    }

    void Item::setTypeId(const ItemData::TypeIdType &id)
    {
        mData.mTypeId = id;
    }

    ItemData::LocationIdType Item::getLocationId() const
    {
        return mData.mLocationId;
    }

    void Item::setLocationId(const ItemData::LocationIdType &id)
    {
        mData.mLocationId = id;
    }

    uint Item::getQuantity() const noexcept
    {
        return mData.mQuantity;
    }

    void Item::setQuantity(uint value) noexcept
    {
        mData.mQuantity = value;
    }

    int Item::getRawQuantity() const noexcept
    {
        return mData.mRawQuantity;
    }

    void Item::setRawQuantity(int value) noexcept
    {
        mData.mRawQuantity = value;
    }

    ItemData Item::getItemData() const &
    {
        return mData;
    }

    ItemData &&Item::getItemData() && noexcept
    {
        return std::move(mData);
    }

    void Item::setItemData(const ItemData &data)
    {
        mData = data;
    }

    void Item::setItemData(ItemData &&data)
    {
        mData = std::move(data);
    }

    Item::ItemIterator Item::begin() noexcept
    {
        return mContents.begin();
    }

    Item::ConstItemIterator Item::begin() const noexcept
    {
        return mContents.begin();
    }

    Item::ItemIterator Item::end() noexcept
    {
        return mContents.end();
    }

    Item::ConstItemIterator Item::end() const noexcept
    {
        return mContents.end();
    }

    Item::CustomValueType Item::getCustomValue() const
    {
        return mCustomValue;
    }

    void Item::setCustomValue(CustomValueType value)
    {
        mCustomValue = std::move(value);
    }

    Item::BPCType Item::getBPCFlag() const noexcept
    {
        return mIsBPC;
    }

    void Item::setBPCFlag(BPCType value) noexcept
    {
        mIsBPC = std::move(value);
    }

    size_t Item::getChildCount() const noexcept
    {
        return mContents.size();
    }

    void Item::addItem(std::unique_ptr<Item> &&item)
    {
        Q_ASSERT(item);

        item->setParentId(getId());
        item->setListId(getListId());
        mContents.emplace_back(std::move(item));
    }

    bool Item::isBPC() const noexcept
    {
        return mIsBPC && *mIsBPC;
    }

    Item &Item::operator =(const Item &other)
    {
        using std::swap;

        Item copy{other};
        swap(static_cast<Entity &>(*this), static_cast<Entity &>(copy));
        swap(mParentId, copy.mParentId);
        swap(mListId, copy.mListId);
        swap(mData, copy.mData);
        swap(mContents, copy.mContents);
        swap(mCustomValue, copy.mCustomValue);

        return *this;
    }
}
