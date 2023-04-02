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

#include <QAbstractTableModel>

#include "MiningLedgerRepository.h"
#include "Character.h"

class QDate;

namespace Evernus
{
    class MiningLedgerRepository;
    class EveDataProvider;

    class MiningLedgerSolarSystemsModel
        : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit MiningLedgerSolarSystemsModel(const EveDataProvider &dataProvider,
                                               const MiningLedgerRepository &ledgerRepo,
                                               QObject *parent = nullptr);
        MiningLedgerSolarSystemsModel(const MiningLedgerSolarSystemsModel &) = default;
        MiningLedgerSolarSystemsModel(MiningLedgerSolarSystemsModel &&) = default;
        virtual ~MiningLedgerSolarSystemsModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void refresh(Character::IdType charId, const QDate &from, const QDate &to);

        MiningLedgerSolarSystemsModel &operator =(const MiningLedgerSolarSystemsModel &) = default;
        MiningLedgerSolarSystemsModel &operator =(MiningLedgerSolarSystemsModel &&) = default;

    private:
        enum
        {
            systemColumn,
            quantityColumn,

            numColumns
        };

        struct AggregatedData
        {
            uint mSolarSystemId = 0;
            quint64 mQuantity = 0;
        };

        const EveDataProvider &mDataProvider;
        const MiningLedgerRepository &mLedgerRepo;

        std::vector<AggregatedData> mData;
    };
}
