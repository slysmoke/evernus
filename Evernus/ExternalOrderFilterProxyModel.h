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

#include <QSortFilterProxyModel>

namespace Evernus
{
    class EveDataProvider;

    class ExternalOrderFilterProxyModel
        : public QSortFilterProxyModel
    {
    public:
        enum SecurityStatus
        {
            NullSec = 0x1,
            LowSec  = 0x2,
            HighSec = 0x4,

            All = NullSec | LowSec | HighSec
        };
        Q_DECLARE_FLAGS(SecurityStatuses, SecurityStatus)

        explicit ExternalOrderFilterProxyModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~ExternalOrderFilterProxyModel() = default;

        virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

        void setFilter(double minPrice, double maxPrice, uint minVolume, uint maxVolume, SecurityStatuses security);

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        const EveDataProvider &mDataProvider;

        double mMinPrice = 0., mMaxPrice = 0.;
        uint mMinVolume = 0, mMaxVolume = 0;
        SecurityStatuses mSecurityStatus = All;
    };

    Q_DECLARE_OPERATORS_FOR_FLAGS(ExternalOrderFilterProxyModel::SecurityStatuses)
}
