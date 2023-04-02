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

#include <QAbstractItemModel>

#include "Contract.h"

namespace Evernus
{
    class EveDataProvider;

    class ContractModel
        : public QAbstractItemModel
    {
        Q_OBJECT

    public:
        typedef std::vector<std::shared_ptr<Contract>> ContractList;

        explicit ContractModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~ContractModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        size_t getNumContracts() const noexcept;
        double getTotalPrice() const noexcept;
        double getTotalReward() const noexcept;
        double getTotalCollateral() const noexcept;
        double getTotalVolume() const noexcept;

        std::shared_ptr<Contract> getContract(const QModelIndex &index) const;

        void reset();

    private slots:
        void updateNames();

    private:
        enum
        {
            issuerColumn,
            issuerCorpColumn,
            assigneeColumn,
            acceptorColumn,
            startStationColumn,
            endStationColumn,
            typeColumn,
            statusColumn,
            titleColumn,
            forCorpColumn,
            availabilityColumn,
            issuedColumn,
            expiredColumn,
            acceptedColumn,
            completedColumn,
            numDaysColumn,
            priceColumn,
            rewardColumn,
            collateralColumn,
            buyoutColumn,
            volumeColumn,

            numColumns
        };

        const EveDataProvider &mDataProvider;

        ContractList mContracts;

        double mTotalPrice = 0.;
        double mTotalReward = 0.;
        double mTotalCollateral = 0.;
        double mTotalVolume = 0.;

        virtual ContractList getContracts() const = 0;

        QVariant contractData(const QModelIndex &index, int role) const;
        QVariant contractItemData(const QModelIndex &index, int role) const;
    };
}
