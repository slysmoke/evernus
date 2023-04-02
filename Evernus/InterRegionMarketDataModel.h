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
#include "EveType.h"

class QColor;

namespace Evernus
{
    class EveDataProvider;
    class ExternalOrder;

    class InterRegionMarketDataModel
        : public QAbstractTableModel
        , public ModelWithTypes
    {
        Q_OBJECT

    public:
        template<class T>
        using TypeMap = std::unordered_map<EveType::IdType, T>;
        template<class T>
        using RegionMap = std::unordered_map<uint, T>;
        using HistoryTypeMap = TypeMap<MarketHistory>;
        using HistoryRegionMap = RegionMap<HistoryTypeMap>;

        explicit InterRegionMarketDataModel(const EveDataProvider &dataProvider, QObject *parent = nullptr);
        virtual ~InterRegionMarketDataModel() = default;

        virtual int columnCount(const QModelIndex &parent = QModelIndex{}) const override;
        virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
        virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
        virtual int rowCount(const QModelIndex &parent = QModelIndex{}) const override;

        void setOrderData(const std::vector<ExternalOrder> &orders,
                          const HistoryRegionMap &history,
                          quint64 srcStation,
                          quint64 dstStation,
                          PriceType srcType,
                          PriceType dstType);
        void setCharacter(const std::shared_ptr<Character> &character);
        void discardBogusOrders(bool flag) noexcept;
        void setBogusOrderThreshold(double value) noexcept;

        virtual EveType::IdType getTypeId(const QModelIndex &index) const override;
        Character::IdType getOwnerId(const QModelIndex &index) const;
        uint getSrcRegionId(const QModelIndex &index) const;
        uint getDstRegionId(const QModelIndex &index) const;

        void reset();

        static int getSrcRegionColumn();
        static int getDstRegionColumn();
        static int getVolumeColumn();
        static int getMarginColumn();

    private:
        enum
        {
            nameColumn,
            scoreColumn,
            srcRegionColumn,
            srcBuyPriceColumn,
            srcSellPriceColumn,
            srcOrderCountColumn,
            dstRegionColumn,
            dstBuyPriceColumn,
            dstSellPriceColumn,
            dstOrderCountColumn,
            differenceColumn,
            volumeColumn,
            marginColumn,

            numColumns
        };

        struct TypeData
        {
            EveType::IdType mId = EveType::invalidId;
            double mSrcBuyPrice = 0.;
            double mSrcSellPrice = 0.;
            double mDstBuyPrice = 0.;
            double mDstSellPrice = 0.;
            double mDifference = 0.;
            double mVolume = 0;
            uint mSrcRegion = 0;
            uint mDstRegion = 0;
            double mMargin = 0.;
            quint64 mSrcBuyOrderCount = 0;
            quint64 mSrcSellOrderCount = 0;
            quint64 mDstBuyOrderCount = 0;
            quint64 mDstSellOrderCount = 0;
        };

        const EveDataProvider &mDataProvider;

        std::vector<TypeData> mData;

        std::shared_ptr<Character> mCharacter;

        bool mDiscardBogusOrders = true;
        double mBogusOrderThreshold = 0.9;

        PriceType mSrcPriceType = PriceType::Buy;
        PriceType mDstPriceType = PriceType::Sell;

        double getSrcPrice(const TypeData &data) const noexcept;
        double getDstPrice(const TypeData &data) const noexcept;
    };
}
