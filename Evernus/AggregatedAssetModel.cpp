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
#include <QLocale>

#include "EveDataProvider.h"
#include "AssetProvider.h"
#include "ExternalOrder.h"
#include "TextUtils.h"
#include "IconUtils.h"
#include "AssetList.h"
#include "Item.h"

#include "AggregatedAssetModel.h"

namespace Evernus
{
    AggregatedAssetModel::AggregatedAssetModel(const AssetProvider &assetProvider,
                                               const EveDataProvider &dataProvider,
                                               QObject *parent)
        : QAbstractTableModel{parent}
        , mAssetProvider{assetProvider}
        , mDataProvider{dataProvider}
    {
    }

    int AggregatedAssetModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant AggregatedAssetModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto &row = mData[index.row()];
        const auto column = index.column();

        switch (role) {
        case Qt::DisplayRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(row.mId);
            case quantityColumn:
                {
                    QLocale locale;
                    return locale.toString(row.mQuantity);
                }
            case totalValue:
                {
                    QLocale locale;
                    return TextUtils::currencyToString(row.mTotalValue, locale);
                }
            }
            break;
        case Qt::UserRole:
            switch (column) {
            case nameColumn:
                return mDataProvider.getTypeName(row.mId);
            case quantityColumn:
                return row.mQuantity;
            case totalValue:
                return row.mTotalValue;
            }
            break;
        case Qt::DecorationRole:
            if (column == nameColumn)
                return row.mDecoration;
        }

        return {};
    }

    QVariant AggregatedAssetModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case quantityColumn:
                return tr("Quantity");
            case totalValue:
                return tr("Total value");
            }
        }

        return {};
    }

    int AggregatedAssetModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void AggregatedAssetModel::setCharacter(Character::IdType id)
    {
        mCharacterId = id;
    }

    void AggregatedAssetModel::setCustomStation(quint64 id)
    {
        mCustomStationId = id;
    }

    void AggregatedAssetModel::setCombineCharacters(bool flag)
    {
        mCombineCharacters = flag;
    }

    bool AggregatedAssetModel::isCombiningCharacters() const
    {
        return mCombineCharacters;
    }

    void AggregatedAssetModel::reset()
    {
        beginResetModel();

        mData.clear();

        ItemMap items;

        if (mCombineCharacters)
        {
            const auto assets = mAssetProvider.fetchAllAssets();
            for (const auto &list : assets)
                fillAssets(list, items);
        }
        else if (Q_LIKELY(mCharacterId != Character::invalidId))
        {
            fillAssets(mAssetProvider.fetchAssetsForCharacter(mCharacterId), items);
        }

        mData.reserve(items.size());

        for (auto &item : items)
        {
            item.second.mId = item.first;
            mData.emplace_back(std::move(item.second));
        }

        endResetModel();
    }

    void AggregatedAssetModel::fillAssets(const std::shared_ptr<AssetList> &assets, ItemMap &map) const
    {
        for (const auto &item : *assets)
        {
            auto itemLocationId = item->getLocationId();
            if (!itemLocationId)
                itemLocationId = 0;

            buildItemMap(*item, map, (mCustomStationId == 0) ? (*itemLocationId) : (mCustomStationId));
        }
    }

    void AggregatedAssetModel::buildItemMap(const Item &item, ItemMap &map, quint64 locationId) const
    {
        const auto typeId = item.getTypeId();
        const auto sellPrice = (item.isBPC()) ? (ExternalOrder::nullOrder()) : (mDataProvider.getTypeStationSellPrice(typeId, locationId));
        const auto quantity = item.getQuantity();
        const auto customValue = item.getCustomValue();
        const auto price = (customValue) ? (*customValue) : (sellPrice->getPrice());

        auto &mappedItem = map[typeId];
        mappedItem.mQuantity += quantity;
        mappedItem.mTotalValue += quantity * price;
        mappedItem.mDecoration = IconUtils::getIconForMetaGroup(mDataProvider.getTypeMetaGroupName(typeId));

        for (const auto &child : item)
            buildItemMap(*child, map, locationId);
    }
}
