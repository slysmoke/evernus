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
#include <QDate>

#include "EveDataProvider.h"
#include "MiningLedger.h"
#include "TextUtils.h"

#include "MiningLedgerModel.h"

namespace Evernus
{
    MiningLedgerModel::MiningLedgerModel(const EveDataProvider &dataProvider,
                                         const MiningLedgerRepository &ledgerRepo,
                                         QObject *parent)
        : QAbstractTableModel{parent}
        , ModelWithTypes{}
        , mDataProvider{dataProvider}
        , mLedgerRepo{ledgerRepo}
    {
    }

    int MiningLedgerModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant MiningLedgerModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        if (role == Qt::DisplayRole)
        {
            const auto &data = mData[index.row()];
            Q_ASSERT(data);

            switch (index.column()) {
            case nameColumn:
                return mDataProvider.getTypeName(data->getTypeId());
            case dateColumn:
                return data->getDate();
            case quantityColumn:
                return QLocale{}.toString(data->getQuantity());
            case solarSystemColumn:
                return mDataProvider.getSolarSystemName(data->getSolarSystemId());
            case profitColumn:
                return TextUtils::currencyToString(getPrice(*data), QLocale{});
            }
        }
        else if (role == Qt::UserRole)
        {
            const auto &data = mData[index.row()];
            Q_ASSERT(data);

            switch (index.column()) {
            case nameColumn:
                return mDataProvider.getTypeName(data->getTypeId());
            case dateColumn:
                return data->getDate();
            case quantityColumn:
                return data->getQuantity();
            case solarSystemColumn:
                return mDataProvider.getSolarSystemName(data->getSolarSystemId());
            case profitColumn:
                return getPrice(*data);
            }
        }

        return {};
    }

    QVariant MiningLedgerModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
        {
            switch (section) {
            case nameColumn:
                return tr("Name");
            case dateColumn:
                return tr("Date");
            case quantityColumn:
                return tr("Quantity");
            case solarSystemColumn:
                return tr("Solar system");
            case profitColumn:
                return tr("Profit");
            }
        }

        return {};
    }

    int MiningLedgerModel::rowCount(const QModelIndex &parent) const
    {
        return (parent.isValid()) ? (0) : (static_cast<int>(mData.size()));
    }

    EveType::IdType MiningLedgerModel::getTypeId(const QModelIndex &index) const
    {
        if (Q_LIKELY(index.isValid()))
        {
            Q_ASSERT(static_cast<int>(mData.size()) > index.row());
            return mData[index.row()]->getTypeId();
        }

        return EveType::invalidId;
    }

    void MiningLedgerModel::refresh(Character::IdType charId, const QDate &from, const QDate &to)
    {
        beginResetModel();

        BOOST_SCOPE_EXIT(this_) {
            this_->endResetModel();
        } BOOST_SCOPE_EXIT_END

        mData = mLedgerRepo.fetchForCharacter(charId, from, to);
    }

    MiningLedgerModel::TypeList MiningLedgerModel::getAllTypes() const
    {
        TypeList result;
        for (const auto &data : mData)
        {
            Q_ASSERT(data);
            result.insert(data->getTypeId());
        }

        return result;
    }

    MiningLedgerModel::SolarSystemList MiningLedgerModel::getAllSolarSystems() const
    {
        SolarSystemList result;
        for (const auto &data : mData)
        {
            Q_ASSERT(data);
            result.insert(data->getSolarSystemId());
        }

        return result;
    }

    void MiningLedgerModel::setOrders(OrderList orders)
    {
        mPriceResolver.setOrders(std::move(orders));
        refreshPrices();
    }

    void MiningLedgerModel::setSellPriceType(PriceType type)
    {
        mPriceResolver.setSellPriceType(type);
        refreshPrices();
    }

    void MiningLedgerModel::setSellStation(quint64 stationId)
    {
        mPriceResolver.setSellStation(stationId);
        refreshPrices();
    }

    void MiningLedgerModel::refreshPrices()
    {
        if (!mData.empty())
        {
            emit dataChanged(index(0, profitColumn), index(static_cast<int>(mData.size()) - 1, profitColumn), { Qt::DisplayRole, Qt::UserRole });
        }
    }

    double MiningLedgerModel::getPrice(const MiningLedger &data) const
    {
        return mPriceResolver.getPrice(data.getTypeId(), data.getQuantity());
    }
}
