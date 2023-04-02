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
#include <limits>

#include <QAbstractItemModel>

#include "Character.h"
#include "EveType.h"

namespace Evernus
{
    class EveDataProvider;
    class ExternalOrder;

    class ExternalOrderModel
        : public QAbstractItemModel
    {
    public:
        enum class DeviationSourceType
        {
            Median,
            Best,
            Cost,
            Fixed
        };

        enum class Grouping
        {
            None,
            Station,
            System,
            Region
        };

        enum class PriceColorMode
        {
            Direction,
            Deviation
        };

        explicit ExternalOrderModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~ExternalOrderModel() = default;

        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex{}) const override;
        virtual QModelIndex parent(const QModelIndex &index) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        virtual int getPriceColumn() const = 0;
        virtual Qt::SortOrder getPriceSortOrder() const = 0;
        virtual int getVolumeColumn() const = 0;

        uint getTotalVolume() const noexcept;
        double getTotalSize() const noexcept;
        double getTotalPrice() const noexcept;
        double getMedianPrice() const noexcept;
        double getMaxPrice() const noexcept;
        double getMinPrice() const noexcept;

        const ExternalOrder &getOrder(std::size_t row) const noexcept;
        EveType::IdType getTypeId() const noexcept;

        virtual void setCharacter(Character::IdType id) = 0;

        void setTypeId(EveType::IdType id) noexcept;
        void setRegionId(uint id) noexcept;
        void setSolarSystemId(uint id) noexcept;
        void setStationId(quint64 id) noexcept;
        void setPriceColorMode(PriceColorMode mode) noexcept;

        Grouping getGrouping() const noexcept;
        void setGrouping(Grouping grouping);

        virtual void reset() = 0;

        void changeDeviationSource(DeviationSourceType type, double value);

    protected:
        struct GroupedData
        {
            quint64 mId = 0;
            double mLowestPrice = std::numeric_limits<double>::max();
            double mMedianPrice = 0.;
            double mHighestPrice = 0.;
            uint mVolumeEntered = 0;
            uint mVolumeRemaining = 0;
            uint mCount = 0;
            double mTotalSize = 0.;
            union
            {
                double mTotalCost = 0.;
                double mTotalProfit;
            };
        };

        static const int groupByColumn = 0;

        const EveDataProvider &mDataProvider;

        Grouping mGrouping = Grouping::None;

        EveType::IdType mTypeId = EveType::invalidId;
        uint mRegionId = 0, mSolarSystemId = 0;
        quint64 mStationId = 0;

        PriceColorMode mPriceColorMode = PriceColorMode::Direction;

        DeviationSourceType mDeviationType = DeviationSourceType::Median;
        double mDeviationValue = 1.;

        double mTotalPrice = 0., mMedianPrice = 0., mMinPrice = 0., mMaxPrice = 0.;
        double mTotalSize = 0.;
        quint64 mTotalVolume = 0;

        std::vector<std::shared_ptr<ExternalOrder>> mOrders;
        std::vector<GroupedData> mGroupedData;

    private:
        QVariant getStationGroupedData(int column, int role, const GroupedData &data) const;
        QVariant getSystemGroupedData(int column, int role, const GroupedData &data) const;
        QVariant getRegionGroupedData(int column, int role, const GroupedData &data) const;

        virtual void refreshGroupedData() = 0;

        virtual QVariant getUngroupedData(int column, int role, const ExternalOrder &order) const = 0;
        virtual QVariant getGenericGroupedData(int column, int role, const GroupedData &data) const = 0;
    };
}
