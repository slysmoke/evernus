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
#include <unordered_map>
#include <unordered_set>

#include "MarketGroupRepository.h"
#include "EveTypeRepository.h"

#include "TradeableTypesTreeModel.h"

namespace Evernus
{
    TradeableTypesTreeModel::TypeItem::TypeItem(const QString &name, EveType::IdType id)
        : mName{name}
        , mId{id}
    {
    }

    void TradeableTypesTreeModel::TypeItem::addChild(std::unique_ptr<TypeItem> &&child)
    {
        Q_ASSERT(child);

        child->mParent = this;
        mChildren.emplace_back(std::move(child));
    }

    TradeableTypesTreeModel::TypeItem *TradeableTypesTreeModel::TypeItem::getChild(int row) const
    {
        return (row < static_cast<int>(mChildren.size())) ? (mChildren[row].get()) : (nullptr);
    }

    TradeableTypesTreeModel::TypeItem *TradeableTypesTreeModel::TypeItem::getParent() const noexcept
    {
        return mParent;
    }

    int TradeableTypesTreeModel::TypeItem::getChildCount() const noexcept
    {
        return static_cast<int>(mChildren.size());
    }

    int TradeableTypesTreeModel::TypeItem::getRow() const
   {
       if (mParent != nullptr)
       {
           auto row = 0;
           for (const auto &child : mParent->mChildren)
           {
               if (child.get() == this)
                   return row;

               ++row;
           }
       }

       return 0;
   }

    QString TradeableTypesTreeModel::TypeItem::getName() const
    {
        return mName;
    }

    EveType::IdType TradeableTypesTreeModel::TypeItem::getId() const noexcept
    {
        return mId;
    }

    template<class T>
    bool TradeableTypesTreeModel::TypeItem::traverseLeafs(T &&callback) const
    {
        if (mChildren.empty())
            return callback(*this);

        for (const auto &child : mChildren)
        {
            if (!child->traverseLeafs(std::forward<T>(callback)))
                return false;
        }

        return true;
    }

    TradeableTypesTreeModel::TradeableTypesTreeModel(const EveTypeRepository &typeRepo,
                                                     const MarketGroupRepository &groupRepo,
                                                     QObject *parent)
        : QAbstractItemModel{parent}
    {
        std::vector<std::unique_ptr<TypeItem>> groupItems;
        std::unordered_map<MarketGroup::IdType, TypeItem *> groupItemMap;
        std::unordered_map<TypeItem *, MarketGroup *> groupMap;

        const auto groups = groupRepo.fetchAll();
        groupItems.reserve(groups.size());

        for (const auto &group : groups)
        {
            auto item = std::make_unique<TypeItem>(group->getName());
            groupItemMap[group->getId()] = item.get();
            groupMap[item.get()] = group.get();

            groupItems.emplace_back(std::move(item));
        }

        std::unordered_set<MarketGroup::IdType> groupsWithTypes;

        const auto types = typeRepo.fetchAllTradeable();
        for (const auto &type : types)
        {
            const auto it = groupItemMap.find(*type->getMarketGroupId());
            if (it == std::end(groupItemMap))
                continue;

            auto groupItem = it->second;
            groupItem->addChild(std::make_unique<TypeItem>(type->getName(), type->getId()));

            do {
                const auto group = groupMap[groupItem];
                if (groupsWithTypes.find(group->getId()) != std::end(groupsWithTypes))
                    break;

                groupsWithTypes.emplace(group->getId());

                if (group->getParentId())
                {
                    const auto it = groupItemMap.find(*group->getParentId());
                    groupItem = (it != std::end(groupItemMap)) ? (it->second) : (nullptr);
                }
                else
                {
                    break;
                }
            } while (groupItem != nullptr);
        }

        for (auto &groupItem : groupItems)
        {
            const auto group = groupMap[groupItem.get()];
            if (groupsWithTypes.find(group->getId()) == std::end(groupsWithTypes))
                continue;

            if (group->getParentId())
            {
                const auto it = groupItemMap.find(*group->getParentId());
                if (it != std::end(groupItemMap))
                    it->second->addChild(std::move(groupItem));
                else
                    mRoot.addChild(std::move(groupItem));
            }
            else
            {
                mRoot.addChild(std::move(groupItem));
            }
        }
    }

    int TradeableTypesTreeModel::columnCount(const QModelIndex &parent) const
    {
        return 1;
    }

    QVariant TradeableTypesTreeModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return QVariant{};

        const auto item = static_cast<const TypeItem *>(index.internalPointer());

        switch (role) {
        case Qt::DisplayRole:
            return item->getName();
        case Qt::CheckStateRole:
            {
                auto state = Qt::Unchecked;
                auto first = true;

                item->traverseLeafs([&state, &first, this](const auto &leaf) {
                    if (mSelectedSet.find(leaf.getId()) != std::end(mSelectedSet))
                    {
                        if (state == Qt::Unchecked)
                        {
                            if (first)
                            {
                                first = false;
                                state = Qt::Checked;
                            }
                            else
                            {
                                state = Qt::PartiallyChecked;
                                return false;
                            }
                        }

                        return true;
                    }

                    if (state == Qt::Checked)
                    {
                        state = Qt::PartiallyChecked;
                        return false;
                    }

                    first = false;

                    return true;
                });

                return state;
            }
        }

        return QVariant{};
    }

    Qt::ItemFlags TradeableTypesTreeModel::flags(const QModelIndex &index) const
    {
        if (!index.isValid())
            return 0;

        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
    }

    QVariant TradeableTypesTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return tr("Name");

        return QVariant{};
    }

    QModelIndex TradeableTypesTreeModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (!hasIndex(row, column, parent))
            return QModelIndex{};

        const TypeItem *parentItem = nullptr;
        if (!parent.isValid())
            parentItem = &mRoot;
        else
            parentItem = static_cast<const TypeItem *>(parent.internalPointer());

        const auto childItem = parentItem->getChild(row);
        return (childItem != nullptr) ? (createIndex(row, column, childItem)) : (QModelIndex{});
    }

    QModelIndex TradeableTypesTreeModel::parent(const QModelIndex &index) const
    {
        if (!index.isValid())
            return QModelIndex{};

        const auto childItem = static_cast<const TypeItem *>(index.internalPointer());
        Q_ASSERT(childItem != nullptr);

        const auto parentItem = childItem->getParent();
        if (parentItem == &mRoot)
            return QModelIndex{};

        return createIndex(parentItem->getRow(), 0, parentItem);
    }

    int TradeableTypesTreeModel::rowCount(const QModelIndex &parent) const
    {
        const TypeItem *parentItem = nullptr;
        if (!parent.isValid())
            parentItem = &mRoot;
        else
            parentItem = static_cast<const TypeItem *>(parent.internalPointer());

        return parentItem->getChildCount();
    }

    bool TradeableTypesTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::CheckStateRole)
        {
            const auto item = static_cast<const TypeItem *>(index.internalPointer());

            std::function<void (const TypeItem &, const QModelIndex &)> uncheck = [=, &uncheck](const TypeItem &item, const QModelIndex &current) {
                const auto id = item.getId();
                if (id == EveType::invalidId)
                {
                    const auto children = item.getChildCount();
                    for (auto i = 0; i < children; ++i)
                        uncheck(*item.getChild(i), this->index(i, 0, current));
                }
                else
                {
                    mSelectedSet.erase(id);
                }

                emit dataChanged(current, current, QVector<int>{} << Qt::CheckStateRole);
            };
            std::function<void (const TypeItem &, const QModelIndex &)> check = [=, &check](const TypeItem &item, const QModelIndex &current) {
                const auto id = item.getId();
                if (id == EveType::invalidId)
                {
                    const auto children = item.getChildCount();
                    for (auto i = 0; i < children; ++i)
                        check(*item.getChild(i), this->index(i, 0, current));
                }
                else
                {
                    mSelectedSet.emplace(id);
                }

                emit dataChanged(current, current, QVector<int>{} << Qt::CheckStateRole);
            };

            switch (value.toInt()) {
            case Qt::Unchecked:
                uncheck(*item, index);
                break;
            case Qt::Checked:
                check(*item, index);
                break;
            default:
                return false;
            }

            auto parentIndex = parent(index);
            while (parentIndex.isValid())
            {
                emit dataChanged(parentIndex, parentIndex, QVector<int>{} << Qt::CheckStateRole);
                parentIndex = parent(parentIndex);
            }

            return true;
        }

        return false;
    }

    TradeableTypesTreeModel::TypeSet TradeableTypesTreeModel::getSelectedTypes() const
    {
        return mSelectedSet;
    }

    void TradeableTypesTreeModel::selectTypes(const TypeSet &types)
    {
        beginResetModel();
        mSelectedSet = types;
        endResetModel();
    }

    void TradeableTypesTreeModel::deselectAll()
    {
        beginResetModel();
        mSelectedSet.clear();
        endResetModel();
    }
}
