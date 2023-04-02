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
#include <boost/scope_exit.hpp>

#include <QLocale>

#include "EveDataProvider.h"
#include "MiningLedger.h"

#include "MiningLedgerSolarSystemsModel.h"

namespace Evernus
{
    MiningLedgerSolarSystemsModel::MiningLedgerSolarSystemsModel(const EveDataProvider &dataProvider,
                                                                 const MiningLedgerRepository &ledgerRepo,
                                                                 QObject *parent)
        : QAbstractTableModel{parent}
        , mDataProvider{dataProvider}
        , mLedgerRepo{ledgerRepo}
    {
    }

    int MiningLedgerSolarSystemsModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant MiningLedgerSolarSystemsModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        if (role == Qt::DisplayRole)
        {
            const auto &data = mData[index.row()];
            switch (index.column()) {
            case systemColumn:
                return mDataProvider.getSolarSystemName(data.mSolarSystemId);
            case quantityColumn:
                return QLocale{}.toString(data.mQuantity);
            }
        }
        else if (role == Qt::UserRole)
        {
            const auto &data = mData[index.row()];
            switch (index.column()) {
            case systemColumn:
                return mDataProvider.getSolarSystemName(data.mSolarSystemId);
            case quantityColumn:
                return data.mQuantity;
            }
        }

        return {};
    }

    QVariant MiningLedgerSolarSystemsModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section) {
            case systemColumn:
                return tr("Solar system");
            case quantityColumn:
                return tr("Quantity");
            }
        }

        return {};
    }

    int MiningLedgerSolarSystemsModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    void MiningLedgerSolarSystemsModel::refresh(Character::IdType charId, const QDate &from, const QDate &to)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        const auto data = mLedgerRepo.fetchSolarSystemsForCharacter(charId, from, to);
        for (const auto &type : data)
            mData.emplace_back(AggregatedData{type.first, type.second});
    }
}
