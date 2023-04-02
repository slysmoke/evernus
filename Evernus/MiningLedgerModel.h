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

#include <unordered_set>

#include <QAbstractTableModel>

#include "MiningLedgerRepository.h"
#include "TypeSellPriceResolver.h"
#include "ModelWithTypes.h"
#include "Character.h"
#include "PriceType.h"
#include "EveType.h"

class QDate;

namespace Evernus
{
    class MiningLedgerRepository;
    class EveDataProvider;

    class MiningLedgerModel
        : public QAbstractTableModel
        , public ModelWithTypes
    {
        Q_OBJECT

    public:
        using TypeList = std::unordered_set<EveType::IdType>;
        using SolarSystemList = std::unordered_set<uint>;

        using OrderList = TypeSellPriceResolver::OrderList;

        explicit MiningLedgerModel(const EveDataProvider &dataProvider,
                                   const MiningLedgerRepository &ledgerRepo,
                                   QObject *parent = nullptr);
        MiningLedgerModel(const MiningLedgerModel &) = default;
        MiningLedgerModel(MiningLedgerModel &&) = default;
        virtual ~MiningLedgerModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        virtual EveType::IdType getTypeId(const QModelIndex &index) const override;

        void refresh(Character::IdType charId, const QDate &from, const QDate &to);
        TypeList getAllTypes() const;
        SolarSystemList getAllSolarSystems() const;

        void setOrders(OrderList orders);
        void setSellPriceType(PriceType type);
        void setSellStation(quint64 stationId);

        MiningLedgerModel &operator =(const MiningLedgerModel &) = default;
        MiningLedgerModel &operator =(MiningLedgerModel &&) = default;

    private:
        enum
        {
            nameColumn,
            dateColumn,
            quantityColumn,
            solarSystemColumn,
            profitColumn,

            numColumns
        };

        const EveDataProvider &mDataProvider;
        const MiningLedgerRepository &mLedgerRepo;

        MiningLedgerRepository::EntityList mData;

        TypeSellPriceResolver mPriceResolver;

        void refreshPrices();
        double getPrice(const MiningLedger &data) const;
    };
}
