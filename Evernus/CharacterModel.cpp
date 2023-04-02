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
#include <QtDebug>

#include "Repository.h"

#include "CharacterModel.h"

namespace Evernus
{
    CharacterModel::CharacterModel(const Repository<Character> &characterRepository, QObject *parent)
        : QAbstractTableModel{parent}
        , mCharacterRepository{characterRepository}
        , mData{mCharacterRepository.fetchAll()}
    {
    }

    Qt::ItemFlags CharacterModel::flags(const QModelIndex &index) const
    {
        auto flags = QAbstractTableModel::flags(index);
        if (index.isValid() && index.column() == 0)
            flags |= Qt::ItemIsUserCheckable;

        return flags;
    }

    QVariant CharacterModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section) {
            case 0:
                return tr("Id");
            case 1:
                return tr("Name");
            }
        }

        return QVariant{};
    }

    int CharacterModel::columnCount(const QModelIndex &parent) const
    {
        return 2;
    }

    QVariant CharacterModel::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant{};

        if (role == Qt::DisplayRole)
        {
            const auto &character = mData[index.row()];
            switch (index.column()) {
            case 0:
                return character->getId();
            case 1:
                return character->getName();
            }
        }
        else if (role == Qt::CheckStateRole && index.column() == 0)
        {
            return (mData[index.row()]->isEnabled()) ? (Qt::Checked) : (Qt::Unchecked);
        }

        return QVariant{};
    }

    bool CharacterModel::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (role == Qt::CheckStateRole && index.isValid() && index.column() == 0)
        {
            mData[index.row()]->setEnabled(value.toInt() == Qt::Checked);
            mCharacterRepository.store(*mData[index.row()]);

            emit dataChanged(index, index, QVector<int>{1, Qt::CheckStateRole});
            return true;
        }

        return false;
    }

    bool CharacterModel::removeRows(int row, int count, const QModelIndex &parent)
    {
		if (row < 0 || row + count > mData.size())
			return false;

        beginRemoveRows(parent, row, row + count);

        for (auto i = row; i < row + count; ++i)
            mCharacterRepository.remove(mData[i]->getId());

        mData.erase(std::next(std::begin(mData), row), std::next(std::begin(mData), row + count));

        endRemoveRows();
        return true;
    }

    int CharacterModel::rowCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return 0;

        return static_cast<int>(mData.size());
    }

    void CharacterModel::reset()
    {
        beginResetModel();
        mData = mCharacterRepository.fetchAll();
        endResetModel();
    }
}
