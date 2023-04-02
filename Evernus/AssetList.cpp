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

#include "AssetList.h"

namespace Evernus
{
    AssetList::AssetList()
        : Entity{}
    {
    }

    AssetList::AssetList(const AssetList &other)
        : Entity{other}
        , mCharacterId{other.mCharacterId}
    {
        for (const auto &item : other)
            addItem(std::make_unique<Item>(*item));
    }

    AssetList::AssetList(ItemList &&items)
        : Entity{}
        , mItems{std::move(items)}
    {
    }

    AssetList::~AssetList()
    {
    }

    Character::IdType AssetList::getCharacterId() const noexcept
    {
        return mCharacterId;
    }

    void AssetList::setCharacterId(Character::IdType id) noexcept
    {
        mCharacterId = id;
    }

    AssetList::Iterator AssetList::begin() noexcept
    {
        return mItems.begin();
    }

    AssetList::ConstIterator AssetList::begin() const noexcept
    {
        return mItems.begin();
    }

    AssetList::Iterator AssetList::end() noexcept
    {
        return mItems.end();
    }

    AssetList::ConstIterator AssetList::end() const noexcept
    {
        return mItems.end();
    }

    size_t AssetList::size() const noexcept
    {
        return mItems.size();
    }

    void AssetList::addItem(ItemType &&item)
    {
        item->setListId(getId());
        mItems.emplace_back(std::move(item));
    }

    AssetList &AssetList::operator =(const AssetList &other)
    {
        using std::swap;

        AssetList temp{other};

        swap(static_cast<Entity &>(*this), static_cast<Entity &>(temp));
        swap(mCharacterId, temp.mCharacterId);
        swap(mItems, temp.mItems);

        return *this;
    }
}
