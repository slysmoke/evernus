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
#include "EveDataProvider.h"

#include "ItemNameModel.h"

namespace Evernus
{
    ItemNameModel::ItemNameModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractListModel{parent}
        , mDataProvider{dataProvider}
    {
    }

    QVariant ItemNameModel::data(const QModelIndex &index, int role) const
    {
        if (Q_LIKELY(index.isValid() && index.column() == 0))
        {
            if (role == Qt::DisplayRole)
                return mDataProvider.getTypeName(mData[index.row()]);
            if (role == Qt::UserRole)
                return mData[index.row()];
        }

        return QVariant{};
    }

    QVariant ItemNameModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (section == 0 && orientation == Qt::Horizontal && role == Qt::DisplayRole)
            return tr("Name");

        return QVariant{};
    }

    bool ItemNameModel::removeRows(int row, int count, const QModelIndex &parent)
    {
        if (parent.isValid())
            return false;

        beginRemoveRows(parent, row, row + count);
        mData.erase(std::next(std::begin(mData), row), std::next(std::begin(mData), row + count));
        endRemoveRows();

        return true;
    }

    int ItemNameModel::rowCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return 0;

        return static_cast<int>(mData.size());
    }

    void ItemNameModel::setTypes(const TypeList &data)
    {
        beginResetModel();
        mData = data;
        endResetModel();
    }

    void ItemNameModel::setTypes(TypeList &&data)
    {
        beginResetModel();
        mData = std::move(data);
        endResetModel();
    }
}
