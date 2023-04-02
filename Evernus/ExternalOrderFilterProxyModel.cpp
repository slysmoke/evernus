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
#include "ExternalOrderModel.h"
#include "EveDataProvider.h"
#include "ExternalOrder.h"

#include "ExternalOrderFilterProxyModel.h"

namespace Evernus
{
    ExternalOrderFilterProxyModel::ExternalOrderFilterProxyModel(const EveDataProvider &dataProvider, QObject *parent)
        : QSortFilterProxyModel{parent}
        , mDataProvider{dataProvider}
    {
    }

    void ExternalOrderFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
    {
        Q_ASSERT(sourceModel == nullptr || dynamic_cast<ExternalOrderModel *>(sourceModel) != nullptr);
        QSortFilterProxyModel::setSourceModel(sourceModel);
    }

    void ExternalOrderFilterProxyModel
    ::setFilter(double minPrice, double maxPrice, uint minVolume, uint maxVolume, SecurityStatuses security)
    {
        mMinPrice = minPrice;
        mMaxPrice = maxPrice;
        mMinVolume = minVolume;
        mMaxVolume = maxVolume;
        mSecurityStatus = security;

        invalidateFilter();
    }

    bool ExternalOrderFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        const auto model = static_cast<const ExternalOrderModel *>(sourceModel());
        const auto &order = model->getOrder(sourceRow);

        if (order.getPrice() < mMinPrice)
            return false;
        if (mMaxPrice > 0.01 && order.getPrice() > mMaxPrice)
            return false;
        if (order.getVolumeRemaining() < mMinVolume)
            return false;
        if (mMaxVolume > 0 && order.getVolumeRemaining() > mMaxVolume)
            return false;

        const auto security = mDataProvider.getSolarSystemSecurityStatus(order.getSolarSystemId());
        if (security < 0.)
            return mSecurityStatus & NullSec;
        if (security < 0.5)
            return mSecurityStatus & LowSec;

        return mSecurityStatus & HighSec;
    }
}
