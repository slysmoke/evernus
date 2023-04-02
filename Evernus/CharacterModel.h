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

#include <QAbstractTableModel>

#include "Character.h"

namespace Evernus
{
    template<class T>
    class Repository;

    class CharacterModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit CharacterModel(const Repository<Character> &characterRepository, QObject *parent = nullptr);
        virtual ~CharacterModel() = default;

        virtual Qt::ItemFlags flags(const QModelIndex &index) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
        virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex{}) override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

    public slots:
        void reset();

    private:
        const Repository<Character> &mCharacterRepository;

        std::vector<std::shared_ptr<Character>> mData;
    };
}
