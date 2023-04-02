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

#include <optional>

#include <QSortFilterProxyModel>

namespace Evernus
{
    class ImportingDataModelProxyModel
        : public QSortFilterProxyModel
    {
    public:
        using QSortFilterProxyModel::QSortFilterProxyModel;

        ImportingDataModelProxyModel() = default;
        ImportingDataModelProxyModel(const ImportingDataModelProxyModel &) = default;
        ImportingDataModelProxyModel(ImportingDataModelProxyModel &&) = default;
        virtual ~ImportingDataModelProxyModel() = default;

        void setFilters(std::optional<double> minAvgVolume,
                        std::optional<double> maxAvgVolume,
                        std::optional<double> minPriceDifference,
                        std::optional<double> maxPriceDifference,
                        std::optional<double> minMargin,
                        std::optional<double> maxMargin);

        ImportingDataModelProxyModel &operator =(const ImportingDataModelProxyModel &) = default;
        ImportingDataModelProxyModel &operator =(ImportingDataModelProxyModel &&) = default;

    protected:
        virtual bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        std::optional<double> mMinAvgVolume;
        std::optional<double> mMaxAvgVolume;
        std::optional<double> mMinPriceDifference;
        std::optional<double> mMaxPriceDifference;
        std::optional<double> mMinMargin;
        std::optional<double> mMaxMargin;
    };
}
