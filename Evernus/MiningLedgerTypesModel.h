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
#include "TypeSellPriceResolver.h"
#include "ModelWithTypes.h"
#include "Character.h"
#include "EveType.h"

class QDate;

namespace Evernus
{
    class MiningLedgerRepository;
    class EveDataProvider;

    class MiningLedgerTypesModel
        : public QAbstractTableModel
        , public ModelWithTypes
    {
        Q_OBJECT

    public:
        using OrderList = TypeSellPriceResolver::OrderList;

        MiningLedgerTypesModel(const EveDataProvider &dataProvider,
                               const MiningLedgerRepository &ledgerRepo,
                               QObject *parent = nullptr);
        MiningLedgerTypesModel(const MiningLedgerTypesModel &) = default;
        MiningLedgerTypesModel(MiningLedgerTypesModel &&) = default;
        virtual ~MiningLedgerTypesModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        virtual EveType::IdType getTypeId(const QModelIndex &index) const override;

        void refresh(Character::IdType charId, const QDate &from, const QDate &to);

        void setOrders(OrderList orders);
        void setSellPriceType(PriceType type);
        void setSellStation(quint64 stationId);

        MiningLedgerTypesModel &operator =(const MiningLedgerTypesModel &) = default;
        MiningLedgerTypesModel &operator =(MiningLedgerTypesModel &&) = default;

    private:
        enum
        {
            nameColumn,
            quantityColumn,
            profitColumn,

            numColumns
        };

        struct AggregatedData
        {
            EveType::IdType mTypeId = EveType::invalidId;
            quint64 mQuantity = 0;
        };

        const EveDataProvider &mDataProvider;
        const MiningLedgerRepository &mLedgerRepo;

        std::vector<AggregatedData> mData;

        TypeSellPriceResolver mPriceResolver;

        void refreshPrices();
        double getPrice(const AggregatedData &data) const;
    };
}
