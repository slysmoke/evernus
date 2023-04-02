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

#include "LeafFilterProxyModel.h"

namespace Evernus
{
    class ContractFilterProxyModel
        : public LeafFilterProxyModel
    {
    public:
        enum StatusFilter
        {
            Outstanding             = 0x001,
            Deleted                 = 0x002,
            Completed               = 0x004,
            Failed                  = 0x008,
            CompletedByIssuer       = 0x010,
            CompletedByContractor   = 0x020,
            Cancelled               = 0x040,
            Rejected                = 0x080,
            Reversed                = 0x100,
            InProgress              = 0x200,

            EveryStatus             = Outstanding | Deleted | Completed | Failed | CompletedByIssuer | CompletedByContractor |
                                      Cancelled | Rejected | Reversed | InProgress
        };
        Q_DECLARE_FLAGS(StatusFilters, StatusFilter)

        static const StatusFilters defaultStatusFilter;

        using LeafFilterProxyModel::LeafFilterProxyModel;
        virtual ~ContractFilterProxyModel() = default;

        virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

        void setStatusFilter(const StatusFilters &filter);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        StatusFilters mStatusFilter = defaultStatusFilter;
    };
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Evernus::ContractFilterProxyModel::StatusFilters)
