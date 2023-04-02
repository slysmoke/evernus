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

#include <QAbstractListModel>

#include "EveType.h"

namespace Evernus
{
    class EveDataProvider;

    class ItemNameModel
        : public QAbstractListModel
    {
        Q_OBJECT

    public:
        typedef std::vector<EveType::IdType> TypeList;

        explicit ItemNameModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex{}) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setTypes(const TypeList &data);
        void setTypes(TypeList &&data);

    private:
        const EveDataProvider &mDataProvider;

        TypeList mData;
    };
}
