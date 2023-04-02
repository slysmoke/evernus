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
#include <algorithm>

#include "EveDataProvider.h"

#include "MarketOrderTreeModel.h"

namespace Evernus
{
    void MarketOrderTreeModel::TreeItem::appendChild(std::unique_ptr<TreeItem> &&child)
    {
        child->mParentItem = this;
        mChildItems.emplace_back(std::move(child));
    }

    void MarketOrderTreeModel::TreeItem::clearChildren()
    {
        mChildItems.clear();
    }

    MarketOrderTreeModel::TreeItem *MarketOrderTreeModel::TreeItem::child(int row) const
    {
        return (row >= static_cast<int>(mChildItems.size())) ? (nullptr) : (mChildItems[row].get());
    }

    int MarketOrderTreeModel::TreeItem::childCount() const
    {
        return static_cast<int>(mChildItems.size());
    }

    const MarketOrder *MarketOrderTreeModel::TreeItem::getOrder() const noexcept
    {
        return mOrder.get();
    }

    void MarketOrderTreeModel::TreeItem::setOrder(const std::shared_ptr<MarketOrder> &order) noexcept
    {
        mOrder = order;
    }

    QString MarketOrderTreeModel::TreeItem::getGroupName() const
    {
        return mGroupName;
    }

    void MarketOrderTreeModel::TreeItem::setGroupName(const QString &name)
    {
        mGroupName = name;
    }

    void MarketOrderTreeModel::TreeItem::setGroupName(QString &&name)
    {
        mGroupName = std::move(name);
    }

    int MarketOrderTreeModel::TreeItem::row() const
    {
        if (mParentItem != nullptr)
        {
            auto row = 0;
            for (const auto &child : mParentItem->mChildItems)
            {
                if (child.get() == this)
                    return row;

                ++row;
            }
        }

        return 0;
    }

    MarketOrderTreeModel::TreeItem *MarketOrderTreeModel::TreeItem::parent() const
    {
        return mParentItem;
    }

    MarketOrderTreeModel::MarketOrderTreeModel(const EveDataProvider &dataProvider,
                                               QObject *parent)
        : MarketOrderModel{parent}
        , mDataProvider{dataProvider}
    {
    }

    QModelIndex MarketOrderTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex();

        const TreeItem *parentItem = nullptr;

        if (!parent.isValid())
            parentItem = &mRootItem;
        else
            parentItem = static_cast<const TreeItem *>(parent.internalPointer());

        auto childItem = parentItem->child(row);
        if (childItem)
            return createIndex(row, column, childItem);

        return QModelIndex{};
    }

    QModelIndex MarketOrderTreeModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QModelIndex{};

        auto childItem = static_cast<const TreeItem *>(index.internalPointer());
        auto parentItem = childItem->parent();

        if (parentItem == &mRootItem)
            return QModelIndex{};

        return createIndex(parentItem->row(), 0, parentItem);
    }

    int MarketOrderTreeModel::rowCount(const QModelIndex &parent) const
    {
        const TreeItem *parentItem = nullptr;
        if (parent.column() > 0)
            return 0;

        if (!parent.isValid())
            parentItem = &mRootItem;
        else
            parentItem = static_cast<const TreeItem *>(parent.internalPointer());

        return parentItem->childCount();
    }

    size_t MarketOrderTreeModel::getOrderCount() const
    {
        return mTotalOrders;
    }

    quint64 MarketOrderTreeModel::getVolumeRemaining() const
    {
        return mVolumeRemaining;
    }

    quint64 MarketOrderTreeModel::getVolumeEntered() const
    {
        return mVolumeEntered;
    }

    double MarketOrderTreeModel::getTotalISK() const
    {
        return mTotalISK;
    }

    double MarketOrderTreeModel::getTotalSize() const
    {
        return mTotalSize;
    }

    MarketOrderModel::Range MarketOrderTreeModel::getOrderRange(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto order = item->getOrder();
        if (order == nullptr)
            return Range{};

        Range range;
        range.mFrom = order->getFirstSeen();
        range.mTo = order->getLastSeen();

        return range;
    }

    EveType::IdType MarketOrderTreeModel::getOrderTypeId(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto order = item->getOrder();
        return (order != nullptr) ? (order->getTypeId()) : (EveType::invalidId);
    }

    Character::IdType MarketOrderTreeModel::getOrderOwnerId(const QModelIndex &index) const
    {
        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        const auto order = item->getOrder();
        return (order != nullptr) ? (order->getCharacterId()) : (Character::invalidId);
    }

    const MarketOrder *MarketOrderTreeModel::getOrder(const QModelIndex &index) const
    {
        if (!index.isValid())
            return nullptr;

        const auto item = static_cast<const TreeItem *>(index.internalPointer());
        return item->getOrder();
    }

    void MarketOrderTreeModel::removeIndexes(const QModelIndexList &indexes)
    {
        std::vector<const MarketOrder *> orders;
        orders.reserve(indexes.size());

        auto sortedIndexes = indexes;
        std::sort(std::begin(sortedIndexes), std::end(sortedIndexes), [](const auto &a, const auto &b) {
            return a.row() > b.row();   // highest to lowest
        });

        for (const auto &index : sortedIndexes)
        {
            const auto order = getOrder(index);
            if (Q_LIKELY(order != nullptr))
                orders.emplace_back(order);
        }

        for (const auto order : orders)
            handleOrderRemoval(*order);

        for (const auto &index : sortedIndexes)
        {
            beginRemoveRows(parent(index), index.row(), index.row() + 1);
            endRemoveRows();
        }
    }

    void MarketOrderTreeModel::setCharacter(Character::IdType id)
    {
        mCharacterId = id;

        handleNewCharacter(mCharacterId);
        reset();
    }

    void MarketOrderTreeModel::setAllCaracters(bool all)
    {
        mAllCharacters = all;

        if (mAllCharacters)
            handleAllCharacters();
        else
            handleNewCharacter(mCharacterId);

        reset();
    }

    void MarketOrderTreeModel::setGrouping(Grouping grouping)
    {
        mGrouping = grouping;
        reset();
    }

    void MarketOrderTreeModel::reset()
    {
        beginResetModel();

        const auto data = (mAllCharacters) ? (getOrdersForAllCharacters()) : (getOrders(mCharacterId));

        mRootItem.clearChildren();

        mTotalOrders = 0;
        mVolumeRemaining = 0;
        mVolumeEntered = 0;
        mTotalISK = 0.;
        mTotalSize = 0.;

        std::unordered_map<quintptr, TreeItem *> groupItems;

        for (const auto &order : data)
        {
            auto item = std::make_unique<TreeItem>();
            item->setOrder(order);

            if (mGrouping != Grouping::None)
            {
                const auto id = getGroupingId(*order);
                auto it = groupItems.find(id);

                if (it == std::end(groupItems))
                {
                    auto item = std::make_unique<TreeItem>();
                    item->setGroupName(getGroupingData(*order));

                    auto itemPtr = item.get();
                    mRootItem.appendChild(std::move(item));

                    it = groupItems.emplace(id, itemPtr).first;
                }

                it->second->appendChild(std::move(item));
            }
            else
            {
                mRootItem.appendChild(std::move(item));
            }

            if (order->getState() != MarketOrder::State::Active)
                continue;

            mVolumeRemaining += order->getVolumeRemaining();
            mVolumeEntered += order->getVolumeEntered();
            mTotalISK += order->getPrice() * order->getVolumeRemaining();
            mTotalSize += mDataProvider.getTypeVolume(order->getTypeId()) * order->getVolumeRemaining();

            ++mTotalOrders;
        }

        endResetModel();
    }

    QString MarketOrderTreeModel::getGroupName(EveType::IdType typeId) const
    {
        auto group = mDataProvider.getTypeMarketGroupParentName(typeId);
        if (group.isEmpty())
            group = mDataProvider.getTypeMarketGroupName(typeId);

        return group;
    }

    void MarketOrderTreeModel::handleNewCharacter(Character::IdType /* characterId */)
    {
    }

    void MarketOrderTreeModel::handleAllCharacters()
    {
    }

    quintptr MarketOrderTreeModel::getGroupingId(const MarketOrder &order) const
    {
        switch (mGrouping) {
        case Grouping::Group:
            return mDataProvider.getTypeMarketGroupParentId(order.getTypeId());
        case Grouping::Station:
            return order.getStationId();
        case Grouping::Type:
            return order.getTypeId();
        default:
            return 0;
        }
    }

    QString MarketOrderTreeModel::getGroupingData(const MarketOrder &order) const
    {
        switch (mGrouping) {
        case Grouping::Group:
            return getGroupName(order.getTypeId());
        case Grouping::Station:
            return mDataProvider.getLocationName(order.getStationId());
        case Grouping::Type:
            return mDataProvider.getTypeName(order.getTypeId());
        default:
            return QString{};
        }
    }
}
