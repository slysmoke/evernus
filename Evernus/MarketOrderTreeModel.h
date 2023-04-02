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

#include "MarketOrderModel.h"
#include "MarketOrder.h"
#include "Character.h"

namespace Evernus
{
    class EveDataProvider;

    class MarketOrderTreeModel
        : public MarketOrderModel
    {
    public:
        explicit MarketOrderTreeModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~MarketOrderTreeModel() = default;

        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        virtual size_t getOrderCount() const override;
        virtual quint64 getVolumeRemaining() const override;
        virtual quint64 getVolumeEntered() const override;
        virtual double getTotalISK() const override;
        virtual double getTotalSize() const override;
        virtual Range getOrderRange(const QModelIndex &index) const override;
        virtual EveType::IdType getOrderTypeId(const QModelIndex &index) const override;
        virtual Character::IdType getOrderOwnerId(const QModelIndex &index) const override;
        virtual const MarketOrder *getOrder(const QModelIndex &index) const override;
        virtual void removeIndexes(const QModelIndexList &indexes) override;

        void setCharacter(Character::IdType id);
        void setAllCaracters(bool all);
        void setGrouping(Grouping grouping);

        void reset();

    protected:
        class TreeItem
        {
        public:
            TreeItem() = default;
            ~TreeItem() = default;

            void appendChild(std::unique_ptr<TreeItem> &&child);
            void clearChildren();

            TreeItem *child(int row) const;

            int childCount() const;

            const MarketOrder *getOrder() const noexcept;
            void setOrder(const std::shared_ptr<MarketOrder> &order) noexcept;

            QString getGroupName() const;
            void setGroupName(const QString &name);
            void setGroupName(QString &&name);

            int row() const;

            TreeItem *parent() const;

        private:
            std::vector<std::unique_ptr<TreeItem>> mChildItems;
            TreeItem *mParentItem = nullptr;
            std::shared_ptr<MarketOrder> mOrder;
            QString mGroupName;
        };

        typedef std::vector<std::shared_ptr<MarketOrder>> OrderList;

        static const auto groupingColumn = 0;

        const EveDataProvider &mDataProvider;

        size_t mTotalOrders = 0;
        quint64 mVolumeRemaining = 0;
        quint64 mVolumeEntered = 0;
        double mTotalISK = 0.;
        double mTotalSize = 0.;

        Grouping mGrouping = Grouping::None;

        TreeItem mRootItem;

        QString getGroupName(EveType::IdType typeId) const;

    private:
        Character::IdType mCharacterId = Character::invalidId;
        bool mAllCharacters = false;

        virtual OrderList getOrders(Character::IdType characterId) const = 0;
        virtual OrderList getOrdersForAllCharacters() const = 0;

        virtual void handleNewCharacter(Character::IdType characterId);
        virtual void handleAllCharacters();
        virtual void handleOrderRemoval(const MarketOrder &order) = 0;

        quintptr getGroupingId(const MarketOrder &order) const;
        QString getGroupingData(const MarketOrder &order) const;
    };
}
