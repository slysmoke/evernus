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
#include "ImportingDataModel.h"

#include "ImportingDataModelProxyModel.h"

namespace Evernus
{
    void ImportingDataModelProxyModel::setFilters(std::optional<double> minAvgVolume,
                                                  std::optional<double> maxAvgVolume,
                                                  std::optional<double> minPriceDifference,
                                                  std::optional<double> maxPriceDifference,
                                                  std::optional<double> minMargin,
                                                  std::optional<double> maxMargin)
    {
        mMinAvgVolume = std::move(minAvgVolume);
        mMaxAvgVolume = std::move(maxAvgVolume);
        mMinPriceDifference = std::move(minPriceDifference);
        mMaxPriceDifference = std::move(maxPriceDifference);
        mMinMargin = std::move(minMargin);
        mMaxMargin = std::move(maxMargin);

        invalidateFilter();
    }

    bool ImportingDataModelProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
    {
        if (!QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
            return false;

        const auto model = sourceModel();
        if (Q_UNLIKELY(model == nullptr))
            return true;

        const auto role = filterRole();

        const auto checkRange = [&, sourceRow, role](auto column, const auto &min, const auto &max) {
            if (!min && !max)
                return true;

            const auto data = model->data(model->index(sourceRow, column, sourceParent), role).toDouble();
            if (min && data < *min)
                return false;
            if (max && data > *max)
                return false;

            return true;
        };

        return checkRange(ImportingDataModel::avgVolumeColumn, mMinAvgVolume, mMaxAvgVolume) &&
               checkRange(ImportingDataModel::priceDifferenceColumn, mMinPriceDifference, mMaxPriceDifference) &&
               checkRange(ImportingDataModel::marginColumn, mMinMargin, mMaxMargin);
    }
}
