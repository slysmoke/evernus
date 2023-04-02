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

#include <unordered_map>
#include <memory>
#include <vector>
#include <map>

#include <QAbstractTableModel>
#include <QDate>

#include "ModelWithTypes.h"
#include "MarketHistory.h"
#include "Character.h"
#include "PriceType.h"

namespace Evernus
{
    class EveDataProvider;
    class ExternalOrder;

    class ImportingDataModel
        : public QAbstractTableModel
        , public ModelWithTypes
    {
        Q_OBJECT

    public:
        enum
        {
            nameColumn,
            avgVolumeColumn,
            medianDstVolume,
            madDstVolume,
            dstVolumeColumn,
            relativeDstVolumeColumn,
            srcOrderCountColumn,
            dstOrderCountColumn,
            srcPriceColumn,
            importPriceColumn,
            dstPriceColumn,
            priceDifferenceColumn,
            marginColumn,
            projectedProfitColumn,

            numColumns
        };

        template<class T>
        using TypeMap = std::unordered_map<EveType::IdType, T>;
        template<class T>
        using RegionMap = std::unordered_map<uint, T>;
        using HistoryTypeMap = TypeMap<MarketHistory>;
        using HistoryRegionMap = RegionMap<HistoryTypeMap>;

        explicit ImportingDataModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        ImportingDataModel(const ImportingDataModel &) = default;
        ImportingDataModel(ImportingDataModel &&) = default;
        virtual ~ImportingDataModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setCharacter(std::shared_ptr<Character> character);
        void discardBogusOrders(bool flag) noexcept;
        void setBogusOrderThreshold(double value) noexcept;

        virtual EveType::IdType getTypeId(const QModelIndex &index) const override;

        void setOrderData(const std::vector<ExternalOrder> &orders,
                          const HistoryRegionMap &history,
                          quint64 srcStation,
                          quint64 dstStation,
                          PriceType srcPriceType,
                          PriceType dstPriceType,
                          int analysisDays,
                          int aggrDays,
                          double pricePerM3,
                          double collateral,
                          PriceType collateralType,
                          bool hideEmptySell);

        void reset();

        ImportingDataModel &operator =(const ImportingDataModel &) = default;
        ImportingDataModel &operator =(ImportingDataModel &&) = default;

    private:
        struct TypeData
        {
            EveType::IdType mId = EveType::invalidId;
            double mAvgVolume = 0.;
            quint64 mMedianVolume = 0;
            double mVolumeMAD = 0.;
            quint64 mDstVolume = 0;
            double mDstPrice = 0.;
            double mSrcPrice = 0.;
            double mImportPrice = 0.;
            double mPriceDifference = 0.;
            double mMargin = 0.;
            double mProjectedProfit = 0.;
            quint64 mSrcOrderCount = 0;
            quint64 mDstOrderCount = 0;
        };

        const EveDataProvider &mDataProvider;

        std::shared_ptr<Character> mCharacter;

        std::vector<TypeData> mData;

        bool mDiscardBogusOrders = true;
        double mBogusOrderThreshold = 0.9;
    };
}
