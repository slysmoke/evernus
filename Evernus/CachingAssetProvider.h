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

#include <unordered_map>

#include "AssetListRepository.h"
#include "AssetProvider.h"
#include "Item.h"

namespace Evernus
{
    class CharacterRepository;
    class ItemRepository;

    class CachingAssetProvider
        : public AssetProvider
    {
    public:
        CachingAssetProvider(const CharacterRepository &characterRepository,
                             const AssetListRepository &assetRepository,
                             const ItemRepository &itemRepository);
        virtual ~CachingAssetProvider() = default;

        virtual AssetPtr fetchAssetsForCharacter(Character::IdType id) const override;
        virtual std::vector<AssetPtr> fetchAllAssets() const override;

        virtual void setCustomValue(Item::IdType id, double value) override;
        virtual void clearCustomValue(Item::IdType id) override;

        void setForCharacter(Character::IdType id, const AssetList &assets);

    private:
        using CharacterAssetMap = std::unordered_map<Character::IdType, AssetListRepository::EntityPtr>;

        const CharacterRepository &mCharacterRepository;
        const AssetListRepository &mAssetRepository;
        const ItemRepository &mItemRepository;

        mutable CharacterAssetMap mAssets;

        Item *findItem(Item::IdType id) const;
    };
}
