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
#include "ContractModel.h"

#include "ContractFilterProxyModel.h"

namespace Evernus
{
    const ContractFilterProxyModel::StatusFilters ContractFilterProxyModel::defaultStatusFilter = EveryStatus;

    void ContractFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
    {
        Q_ASSERT(dynamic_cast<ContractModel *>(sourceModel) != nullptr);
        LeafFilterProxyModel::setSourceModel(sourceModel);
    }

    void ContractFilterProxyModel::setStatusFilter(const StatusFilters &filter)
    {
        mStatusFilter = filter;
        invalidateFilter();
    }

    bool ContractFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (!LeafFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        const auto model = sourceModel();
        const auto contract = static_cast<const ContractModel *>(model)->getContract(model->index(sourceRow, 0, sourceParent));
        if (!contract)
            return true;

        switch (contract->getStatus()) {
        case Contract::Status::Cancelled:
            return mStatusFilter & Cancelled;
        case Contract::Status::Completed:
            return mStatusFilter & Completed;
        case Contract::Status::CompletedByContractor:
            return mStatusFilter & CompletedByContractor;
        case Contract::Status::CompletedByIssuer:
            return mStatusFilter & CompletedByIssuer;
        case Contract::Status::Deleted:
            return mStatusFilter & Deleted;
        case Contract::Status::Failed:
            return mStatusFilter & Failed;
        case Contract::Status::InProgress:
            return mStatusFilter & InProgress;
        case Contract::Status::Outstanding:
            return mStatusFilter & Outstanding;
        case Contract::Status::Rejected:
            return mStatusFilter & Rejected;
        case Contract::Status::Reversed:
            return mStatusFilter & Reversed;
        }

        return false;
    }
}
