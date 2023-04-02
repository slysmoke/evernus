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
#include <functional>

#include "CharacterRepository.h"
#include "ItemRepository.h"

#include "CachingAssetProvider.h"

namespace Evernus
{
    CachingAssetProvider::CachingAssetProvider(const CharacterRepository &characterRepository,
                                               const AssetListRepository &assetRepository,
                                               const ItemRepository &itemRepository)
        : AssetProvider{}
        , mCharacterRepository{characterRepository}
        , mAssetRepository{assetRepository}
        , mItemRepository{itemRepository}
    {
    }

    CachingAssetProvider::AssetPtr CachingAssetProvider::fetchAssetsForCharacter(Character::IdType id) const
    {
        if (id == Character::invalidId)
           return std::make_shared<AssetList>();

        const auto it = mAssets.find(id);
        if (it != std::end(mAssets))
           return it->second;

        AssetListRepository::EntityPtr assets;

        try
        {
           assets = mAssetRepository.fetchForCharacter(id);
        }
        catch (const AssetListRepository::NotFoundException &)
        {
           assets = std::make_shared<AssetList>();
           assets->setCharacterId(id);
        }

        mAssets.emplace(id, assets);
        return assets;
    }

    std::vector<CachingAssetProvider::AssetPtr> CachingAssetProvider::fetchAllAssets() const
    {
        std::vector<AssetPtr> result;

        const auto idName = mCharacterRepository.getIdColumn();
        auto query = mCharacterRepository.getEnabledQuery();

        while (query.next())
            result.emplace_back(fetchAssetsForCharacter(query.value(idName).value<Character::IdType>()));

        return result;
    }

    void CachingAssetProvider::setCustomValue(Item::IdType id, double value)
    {
        mItemRepository.setCustomValue(id, value);

        const auto item = findItem(id);
        if (item != nullptr)
            item->setCustomValue(value);
    }

    void CachingAssetProvider::clearCustomValue(Item::IdType id)
    {
        mItemRepository.clearCustomValue(id);

        const auto item = findItem(id);
        if (item != nullptr)
            item->setCustomValue(Item::CustomValueType{});
    }

    void CachingAssetProvider::setForCharacter(Character::IdType id, const AssetList &assets)
    {
        const auto it = mAssets.find(id);
        if (it != std::end(mAssets))
            *it->second = assets;
    }

    Item *CachingAssetProvider::findItem(Item::IdType id) const
    {
        const std::function<Item *(Item &)> lookForItem = [&](Item &item) -> Item * {
            if (item.getId() == id)
                return &item;

            for (const auto &child : item)
            {
                const auto item = lookForItem(*child);
                if (item != nullptr)
                    return item;
            }

            return nullptr;
        };

        for (const auto &assets : mAssets)
        {
            for (const auto &item : *assets.second)
            {
                const auto found = lookForItem(*item);
                if (found != nullptr)
                    return found;
            }
        }

        return nullptr;
    }
}
