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
#include "EveDataProvider.h"

#include "ExternalOrderModel.h"

namespace Evernus
{
    ExternalOrderModel::ExternalOrderModel(const EveDataProvider &dataProvider, QObject *parent)
        : QAbstractItemModel{parent}
        , mDataProvider{dataProvider}
    {
    }

    QVariant ExternalOrderModel::data(const QModelIndex &index, int role) const
    {
        if (Q_UNLIKELY(!index.isValid()))
            return {};

        const auto column = index.column();

        switch (mGrouping) {
        case Grouping::None:
            return getUngroupedData(column, role, *mOrders[index.row()]);
        case Grouping::Station:
            return getStationGroupedData(column, role, mGroupedData[index.row()]);
        case Grouping::System:
            return getSystemGroupedData(column, role, mGroupedData[index.row()]);
        case Grouping::Region:
            return getRegionGroupedData(column, role, mGroupedData[index.row()]);
        }

        return {};
    }

    QModelIndex ExternalOrderModel::index(int row, int column, const QModelIndex &parent) const
    {
        if (!parent.isValid())
            return createIndex(row, column);

        return {};
    }

    QModelIndex ExternalOrderModel::parent(const QModelIndex &index) const
    {
        Q_UNUSED(index);
        return {};
    }

    int ExternalOrderModel::rowCount(const QModelIndex &parent) const
    {
        if (parent.isValid())
            return 0;

        return (mGrouping == Grouping::None) ? (static_cast<int>(mOrders.size())) : (static_cast<int>(mGroupedData.size()));
    }

    uint ExternalOrderModel::getTotalVolume() const noexcept
    {
        return mTotalVolume;
    }

    double ExternalOrderModel::getTotalSize() const noexcept
    {
        return mTotalSize;
    }

    double ExternalOrderModel::getTotalPrice() const noexcept
    {
        return mTotalPrice;
    }

    double ExternalOrderModel::getMedianPrice() const noexcept
    {
        return mMedianPrice;
    }

    double ExternalOrderModel::getMaxPrice() const noexcept
    {
        return mMaxPrice;
    }

    double ExternalOrderModel::getMinPrice() const noexcept
    {
        return mMinPrice;
    }

    const ExternalOrder &ExternalOrderModel::getOrder(std::size_t row) const noexcept
    {
        return *mOrders[row];
    }

    EveType::IdType ExternalOrderModel::getTypeId() const noexcept
    {
        return mTypeId;
    }

    void ExternalOrderModel::setTypeId(EveType::IdType id) noexcept
    {
        mTypeId = id;
    }

    void ExternalOrderModel::setRegionId(uint id) noexcept
    {
        mRegionId = id;
    }

    void ExternalOrderModel::setSolarSystemId(uint id) noexcept
    {
        mSolarSystemId = id;
    }

    void ExternalOrderModel::setStationId(quint64 id) noexcept
    {
        mStationId = id;
    }

    void ExternalOrderModel::setPriceColorMode(PriceColorMode mode) noexcept
    {
        mPriceColorMode = mode;
    }

    ExternalOrderModel::Grouping ExternalOrderModel::getGrouping() const noexcept
    {
        return mGrouping;
    }

    void ExternalOrderModel::setGrouping(Grouping grouping)
    {
        beginResetModel();

        mGrouping = grouping;
        refreshGroupedData();

        endResetModel();
    }

    void ExternalOrderModel::changeDeviationSource(DeviationSourceType type, double value)
    {
        beginResetModel();

        mDeviationType = type;
        mDeviationValue = value;

        endResetModel();
    }

    QVariant ExternalOrderModel::getStationGroupedData(int column, int role, const GroupedData &data) const
    {
        if (column == groupByColumn)
        {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::UserRole:
                return mDataProvider.getLocationName(data.mId);
            }

            return {};
        }

        return getGenericGroupedData(column, role, data);
    }

    QVariant ExternalOrderModel::getSystemGroupedData(int column, int role, const GroupedData &data) const
    {
        if (column == groupByColumn)
        {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::UserRole:
                return mDataProvider.getSolarSystemName(data.mId);
            }

            return {};
        }

        return getGenericGroupedData(column, role, data);
    }

    QVariant ExternalOrderModel::getRegionGroupedData(int column, int role, const GroupedData &data) const
    {
        if (column == groupByColumn)
        {
            switch (role) {
            case Qt::DisplayRole:
            case Qt::UserRole:
                return mDataProvider.getRegionName(data.mId);
            }

            return {};
        }

        return getGenericGroupedData(column, role, data);
    }
}
