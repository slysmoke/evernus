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

#include <QAbstractTableModel>

#include "ItemCostProvider.h"

namespace Evernus
{
    class EveDataProvider;

    class ItemCostModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        ItemCostModel(const ItemCostProvider &costProvider,
                      const EveDataProvider &dataProvider,
                      QObject *parent = nullptr);
        virtual ~ItemCostModel() = default;

        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex{}) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setCharacter(Character::IdType id);

        void reset();

        ItemCost::IdType getId(int row) const;
        EveType::IdType getTypeId(int row) const;
        double getCost(int row) const;

    private:
        const ItemCostProvider &mCostProvider;
        const EveDataProvider &mDataProvider;

        Character::IdType mCharacterId = Character::invalidId;

        ItemCostProvider::CostList mData;
    };
}
