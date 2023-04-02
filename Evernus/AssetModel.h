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
#include <optional>

#include <QAbstractItemModel>
#include <QDateTime>

#include "Character.h"
#include "Item.h"

namespace Evernus
{
    class EveDataProvider;
    class AssetProvider;
    class AssetList;

    class AssetModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        using LocationId = ItemData::LocationIdType::value_type;
        using CustomValueType = std::optional<double>;

        AssetModel(const AssetProvider &assetProvider,
                   const EveDataProvider &dataProvider,
                   bool showOwner,
                   QObject *parent = nullptr);
        virtual ~AssetModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setCharacter(Character::IdType id);
        void setCustomStation(quint64 id);

        void setCombineCharacters(bool flag);
        bool isCombiningCharacters() const;

        void reset();

        uint getTotalAssets() const noexcept;
        double getTotalVolume() const noexcept;
        double getTotalSellPrice() const noexcept;

        LocationId getAssetLocationId(const QModelIndex &index) const;
        ItemData::TypeIdType getAssetTypeId(const QModelIndex &index) const;
        Character::IdType getAssetOwnerId(const QModelIndex &index) const;
        CustomValueType getAssetCustomValue(const QModelIndex &index) const;
        Item::IdType getAssetId(const QModelIndex &index) const;

    private slots:
        void updateNames();

    private:
        class TreeItem final
        {
        public:
            TreeItem() = default;
            ~TreeItem() = default;

            void appendChild(std::unique_ptr<TreeItem> child);
            void clearChildren() noexcept;

            TreeItem *child(int row) const noexcept;

            int childCount() const noexcept;
            int columnCount() const;

            QVariant data(int column) const;
            QVariantList data() const;
            void setData(const QVariantList &data);
            void addData(const QVariant &data);

            QDateTime priceTimestamp() const;
            void setPriceTimestamp(const QDateTime &dt);

            LocationId locationId() const noexcept;
            void setLocationId(LocationId id) noexcept;

            ItemData::TypeIdType typeId() const noexcept;
            void setTypeId(ItemData::TypeIdType id) noexcept;

            Character::IdType ownerId() const noexcept;
            void setOwnerId(Character::IdType id) noexcept;

            QVariant decoration() const;
            void setDecoration(const QVariant &data);

            CustomValueType customValue() const;
            void setCustomValue(CustomValueType value);

            Item::IdType id() const noexcept;
            void setId(Item::IdType id) noexcept;

            int row() const noexcept;

            TreeItem *parent() const noexcept;

        private:
            std::vector<std::unique_ptr<TreeItem>> mChildItems;
            QVariantList mItemData;
            QDateTime mPriceTimestamp;
            LocationId mLocationId = LocationId{};
            ItemData::TypeIdType mTypeId = ItemData::TypeIdType{};
            Character::IdType mOwnerId = Character::IdType{};
            QVariant mDecoration;
            CustomValueType mCustomValue;
            Item::IdType mId = Item::invalidId;
            TreeItem *mParentItem = nullptr;
        };

        enum
        {
            typeColumn,
            quantityColumn,
            unitVolumeColumn,
            totalVolumeColumn,
            unitPriceColumn,
            customValueColumn,
            totalPriceColumn,
            ownerColumn,

            numColumns
        };

        const AssetProvider &mAssetProvider;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;
        quint64 mCustomStationId = 0;
        bool mCombineCharacters = false;

        TreeItem mRootItem;

        uint mTotalAssets = 0;
        double mTotalVolume = 0.;
        double mTotalSellPrice = 0.;

        std::unordered_map<LocationId, TreeItem *> mLocationItems;

        void buildItemMap(const Item &item, TreeItem &treeItem, LocationId locationId, Character::IdType ownerId);

        std::unique_ptr<TreeItem> createTreeItemForItem(const Item &item, LocationId locationId, Character::IdType ownerId);
        void fillAssets(const std::shared_ptr<AssetList> &assets);
    };
}
