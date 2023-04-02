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
#include "TextUtils.h"

#include "MiningLedgerTypesModel.h"

namespace Evernus
{
    MiningLedgerTypesModel::MiningLedgerTypesModel(const EveDataProvider &dataProvider,
                                                   const MiningLedgerRepository &ledgerRepo,
                                                   QObject *parent)
        : QAbstractTableModel{parent}
        , ModelWithTypes{}
        , mDataProvider{dataProvider}
        , mLedgerRepo{ledgerRepo}
    {
    }

    int MiningLedgerTypesModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant MiningLedgerTypesModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        if (role == Qt::DisplayRole)
        {
            const auto &data = mData[index.row()];
            switch (index.column()) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mTypeId);
            case quantityColumn:
                return QLocale{}.toString(data.mQuantity);
            case profitColumn:
                return TextUtils::currencyToString(getPrice(data), QLocale{});
            }
        }
        else if (role == Qt::UserRole)
        {
            const auto &data = mData[index.row()];
            switch (index.column()) {
            case nameColumn:
                return mDataProvider.getTypeName(data.mTypeId);
            case quantityColumn:
                return data.mQuantity;
            case profitColumn:
                return getPrice(data);
            }
        }

        return {};
    }

    QVariant MiningLedgerTypesModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case quantityColumn:
                return tr("Quantity");
            case profitColumn:
                return tr("Profit");
            }
        }

        return {};
    }

    int MiningLedgerTypesModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    EveType::IdType MiningLedgerTypesModel::getTypeId(const QModelIndex &index) const
    {
        if (Q_LIKELY(index.isValid()))
        {
            Q_ASSERT(static_cast<int>(mData.size()) > index.row());
            return mData[index.row()].mTypeId;
        }

        return EveType::invalidId;
    }

    void MiningLedgerTypesModel::refresh(Character::IdType charId, const QDate &from, const QDate &to)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData.clear();

        const auto data = mLedgerRepo.fetchTypesForCharacter(charId, from, to);
        for (const auto &type : data)
            mData.emplace_back(AggregatedData{type.first, type.second});
    }

    void MiningLedgerTypesModel::setOrders(OrderList orders)
    {
        mPriceResolver.setOrders(std::move(orders));
        refreshPrices();
    }

    void MiningLedgerTypesModel::setSellPriceType(PriceType type)
    {
        mPriceResolver.setSellPriceType(type);
        refreshPrices();
    }

    void MiningLedgerTypesModel::setSellStation(quint64 stationId)
    {
        mPriceResolver.setSellStation(stationId);
        refreshPrices();
    }

    void MiningLedgerTypesModel::refreshPrices()
    {
        if (!mData.empty())
        {
            emit dataChanged(index(0, profitColumn), index(static_cast<int>(mData.size()) - 1, profitColumn), { Qt::DisplayRole, Qt::UserRole });
        }
    }

    double MiningLedgerTypesModel::getPrice(const AggregatedData &data) const
    {
        return mPriceResolver.getPrice(data.mTypeId, data.mQuantity);
    }
}
