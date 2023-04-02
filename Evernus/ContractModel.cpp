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
#include <QLocale>

#include "EveDataProvider.h"
#include "TextUtils.h"

#include "ContractModel.h"

namespace Evernus
{
    ContractModel::ContractModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractItemModel{parent}
        , mDataProvider{dataProvider}
    {
        connect(&mDataProvider, &EveDataProvider::namesChanged, this, &ContractModel::updateNames);
    }

    int ContractModel::columnCount(const QModelIndex &parent) const
    {
        Q_UNUSED(parent);
        return numColumns;
    }

    QVariant ContractModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return QVariant{};

        return (index.internalId() == 0) ? (contractData(index, role)) : (contractItemData(index, role));
    }

    QVariant ContractModel::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        {
            switch (section) {
            case issuerColumn:
                return tr("Issuer");
            case issuerCorpColumn:
                return tr("Issuer corporation");
            case assigneeColumn:
                return tr("Assignee");
            case acceptorColumn:
                return tr("Acceptor");
            case startStationColumn:
                return tr("Start station");
            case endStationColumn:
                return tr("End station");
            case typeColumn:
                return tr("Type");
            case statusColumn:
                return tr("Status");
            case titleColumn:
                return tr("Title");
            case forCorpColumn:
                return tr("Corporation");
            case availabilityColumn:
                return tr("Availability");
            case issuedColumn:
                return tr("Issued");
            case expiredColumn:
                return tr("Expiration");
            case acceptedColumn:
                return tr("Accepted");
            case completedColumn:
                return tr("Completed");
            case numDaysColumn:
                return tr("Days");
            case priceColumn:
                return tr("Price");
            case rewardColumn:
                return tr("Reward");
            case collateralColumn:
                return tr("Collateral");
            case buyoutColumn:
                return tr("Buyout");
            case volumeColumn:
                return tr("Volume");
            }
        }

        return QVariant{};
    }

    QModelIndex ContractModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (parent.isValid())
        {
            if (parent.internalId() != 0)
                return QModelIndex{};

            return createIndex(row, column, parent.row() + 1);
        }

        return createIndex(row, column);
    }

    QModelIndex ContractModel::parent(const QModelIndex &index) const
    {
        const auto id = index.internalId();
        if (id == 0)
            return QModelIndex{};

        return createIndex(id - 1, 0);
    }

    int ContractModel::rowCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return (parent.internalId() == 0) ? (static_cast<int>(mContracts[parent.row()]->getItemCount())) : (0);

        return static_cast<int>(mContracts.size());
    }

    size_t ContractModel::getNumContracts() const noexcept
    {
        return mContracts.size();
    }

    double ContractModel::getTotalPrice() const noexcept
    {
        return mTotalPrice;
    }

    double ContractModel::getTotalReward() const noexcept
    {
        return mTotalReward;
    }

    double ContractModel::getTotalCollateral() const noexcept
    {
        return mTotalCollateral;
    }

    double ContractModel::getTotalVolume() const noexcept
    {
        return mTotalVolume;
    }

    std::shared_ptr<Contract> ContractModel::getContract(const QModelIndex &index) const
    {
        return (index.internalId() == 0) ? (mContracts[index.row()]) : (std::shared_ptr<Contract>{});
    }

    void ContractModel::reset()
    {
        beginResetModel();

        mTotalPrice = mTotalReward = mTotalCollateral = mTotalVolume = 0.;

        mContracts = getContracts();
        for (const auto &contract : mContracts)
        {
            const auto status = contract->getStatus();
            if (status != Contract::Status::InProgress && status != Contract::Status::Outstanding)
                continue;

            mTotalPrice += contract->getPrice();
            mTotalReward += contract->getReward();
            mTotalCollateral += contract->getCollateral();
            mTotalVolume += contract->getVolume();
        }

        endResetModel();
    }

    void ContractModel::updateNames()
    {
        emit dataChanged(index(0, issuerColumn),
                         index(rowCount() - 1, acceptorColumn),
                         QVector<int>{}
                             << Qt::UserRole
                             << Qt::DisplayRole);
    }

    QVariant ContractModel::contractData(const QModelIndex &index, int role) const
    {
        const auto &contract = mContracts[index.row()];

        switch (role) {
        case Qt::UserRole:
            switch (index.column()) {
            case issuerColumn:
                return mDataProvider.getGenericName(contract->getIssuerId());
            case issuerCorpColumn:
                return mDataProvider.getGenericName(contract->getIssuerCorpId());
            case assigneeColumn:
                return mDataProvider.getGenericName(contract->getAssigneeId());
            case acceptorColumn:
                return mDataProvider.getGenericName(contract->getAcceptorId());
            case startStationColumn:
                return mDataProvider.getLocationName(contract->getStartStationId());
            case endStationColumn:
                return mDataProvider.getLocationName(contract->getEndStationId());
            case typeColumn:
                return static_cast<int>(contract->getType());
            case statusColumn:
                return static_cast<int>(contract->getStatus());
            case titleColumn:
                return contract->getTitle();
            case forCorpColumn:
                return contract->isForCorp();
            case availabilityColumn:
                return static_cast<int>(contract->getAvailability());
            case issuedColumn:
                return contract->getIssued();
            case expiredColumn:
                return contract->getExpired();
            case acceptedColumn:
                return contract->getAccepted();
            case completedColumn:
                return contract->getCompleted();
            case numDaysColumn:
                return contract->getNumDays();
            case priceColumn:
                return contract->getPrice();
            case rewardColumn:
                return contract->getReward();
            case collateralColumn:
                return contract->getCollateral();
            case buyoutColumn:
                return contract->getBuyout();
            case volumeColumn:
                return contract->getVolume();
            }
            break;
        case Qt::DisplayRole:
            {
                QLocale locale;

                switch (index.column()) {
                case issuerColumn:
                    return mDataProvider.getGenericName(contract->getIssuerId());
                case issuerCorpColumn:
                    return mDataProvider.getGenericName(contract->getIssuerCorpId());
                case assigneeColumn:
                    return mDataProvider.getGenericName(contract->getAssigneeId());
                case acceptorColumn:
                    return mDataProvider.getGenericName(contract->getAcceptorId());
                case startStationColumn:
                    return mDataProvider.getLocationName(contract->getStartStationId());
                case endStationColumn:
                    return mDataProvider.getLocationName(contract->getEndStationId());
                case typeColumn:
                    switch (contract->getType()) {
                    case Contract::Type::Unknown:
                        return tr("Unknown");
                    case Contract::Type::ItemExchange:
                        return tr("Item Exchange");
                    case Contract::Type::Courier:
                        return tr("Courier");
                    case Contract::Type::Auction:
                        return tr("Auction");
                    case Contract::Type::Loan:
                        return tr("Loan");
                    }
                    break;
                case statusColumn:
                    switch (contract->getStatus()) {
                    case Contract::Status::Outstanding:
                        return tr("Outstanding");
                    case Contract::Status::Deleted:
                        return tr("Deleted");
                    case Contract::Status::Completed:
                        return tr("Completed");
                    case Contract::Status::Failed:
                        return tr("Failed");
                    case Contract::Status::CompletedByIssuer:
                        return tr("Completed by Issuer");
                    case Contract::Status::CompletedByContractor:
                        return tr("Completed by Contractor");
                    case Contract::Status::Cancelled:
                        return tr("Cancelled");
                    case Contract::Status::Rejected:
                        return tr("Rejected");
                    case Contract::Status::Reversed:
                        return tr("Reversed");
                    case Contract::Status::InProgress:
                        return tr("In Progress");
                    }
                    break;
                case titleColumn:
                    return contract->getTitle();
                case forCorpColumn:
                    return (contract->isForCorp()) ? (tr("yes")) : (tr("no"));
                case availabilityColumn:
                    switch (contract->getAvailability()) {
                    case Contract::Availability::Private:
                        return tr("Private");
                    case Contract::Availability::Public:
                        return tr("Public");
                    case Contract::Availability::Corporation:
                        return tr("Corporation");
                    case Contract::Availability::Alliance:
                        return tr("Alliance");
                    }
                    break;
                case issuedColumn:
                    return TextUtils::dateTimeToString(contract->getIssued().toLocalTime(), locale);
                case expiredColumn:
                    return TextUtils::dateTimeToString(contract->getExpired().toLocalTime(), locale);
                case acceptedColumn:
                    return TextUtils::dateTimeToString(contract->getAccepted().toLocalTime(), locale);
                case completedColumn:
                    return TextUtils::dateTimeToString(contract->getCompleted().toLocalTime(), locale);
                case numDaysColumn:
                    return locale.toString(contract->getNumDays());
                case priceColumn:
                    return TextUtils::currencyToString(contract->getPrice(), locale);
                case rewardColumn:
                    return TextUtils::currencyToString(contract->getReward(), locale);
                case collateralColumn:
                    return TextUtils::currencyToString(contract->getCollateral(), locale);
                case buyoutColumn:
                    return TextUtils::currencyToString(contract->getBuyout(), locale);
                case volumeColumn:
                    return QString{"%1m³"}.arg(locale.toString(contract->getVolume(), 'f', 2));
                }
            }
        }

        return QVariant{};
    }

    QVariant ContractModel::contractItemData(const QModelIndex &index, int role) const
    {
        if ((role == Qt::DisplayRole || role == Qt::UserRole) && (index.column() == 0))
        {
            const auto &contract = mContracts[index.internalId() - 1];
            const auto item = contract->getItem(index.row());

            return QString{"%1 %2%3"}.arg(mDataProvider.getTypeName(item->getTypeId())).arg(QChar(215)).arg(QLocale{}.toString(item->getQuantity()));
        }

        return QVariant{};
    }
}
