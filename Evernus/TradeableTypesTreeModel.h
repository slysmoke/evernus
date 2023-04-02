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

#include <unordered_set>
#include <memory>
#include <vector>

#include <QAbstractItemModel>

#include "EveType.h"

namespace Evernus
{
    class MarketGroupRepository;
    class EveTypeRepository;

    class TradeableTypesTreeModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        using TypeSet = std::unordered_set<EveType::IdType>;

        TradeableTypesTreeModel(const EveTypeRepository &typeRepo,
                                const MarketGroupRepository &groupRepo,
                                QObject *parent = nullptr);
        virtual ~TradeableTypesTreeModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

        TypeSet getSelectedTypes() const;
        void selectTypes(const TypeSet &types);
        void deselectAll();

    private:
        class TypeItem
        {
        public:
            TypeItem() = default;
            explicit TypeItem(const QString &name, EveType::IdType id = EveType::invalidId);

            void addChild(std::unique_ptr<TypeItem> &&child);
            TypeItem *getChild(int row) const;
            TypeItem *getParent() const noexcept;

            int getChildCount() const noexcept;
            int getRow() const;

            QString getName() const;
            EveType::IdType getId() const noexcept;

            template<class T>
            bool traverseLeafs(T &&callback) const;

        private:
            QString mName;
            EveType::IdType mId = 0;
            TypeItem *mParent = nullptr;

            std::vector<std::unique_ptr<TypeItem>> mChildren;
        };

        TypeItem mRoot;

        TypeSet mSelectedSet;
    };
}
