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
#include <vector>
#include <memory>

#include <QAbstractTableModel>
#include <QIcon>

#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class EveDataProvider;
    class AssetProvider;
    class AssetList;
    class Item;

    class AggregatedAssetModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        AggregatedAssetModel(const AssetProvider &assetProvider,
                             const EveDataProvider &dataProvider,
                             QObject *parent = nullptr);
        AggregatedAssetModel(const AggregatedAssetModel &) = default;
        AggregatedAssetModel(AggregatedAssetModel &&) = default;
        virtual ~AggregatedAssetModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setCharacter(Character::IdType id);
        void setCustomStation(quint64 id);

        void setCombineCharacters(bool flag);
        bool isCombiningCharacters() const;

        void reset();

        AggregatedAssetModel &operator =(const AggregatedAssetModel &) = default;
        AggregatedAssetModel &operator =(AggregatedAssetModel &&) = default;

    private:
        enum
        {
            nameColumn,
            quantityColumn,
            totalValue,

            numColumns
        };

        struct ItemData
        {
            EveType::IdType mId = EveType::invalidId;
            quint64 mQuantity = 0;
            double mTotalValue = 0.;
            QIcon mDecoration;
        };

        using ItemMap = std::unordered_map<EveType::IdType, ItemData>;

        const AssetProvider &mAssetProvider;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;
        quint64 mCustomStationId = 0;
        bool mCombineCharacters = false;

        std::vector<ItemData> mData;

        void fillAssets(const std::shared_ptr<AssetList> &assets, ItemMap &map) const;
        void buildItemMap(const Item &item, ItemMap &map, quint64 locationId) const;
    };
}
