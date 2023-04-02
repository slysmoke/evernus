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

#include "TypeAggregatedMarketDataFilterProxyModel.h"
#include "InterRegionMarketDataFilterProxyModel.h"
#include "TypeAggregatedMarketDataModel.h"
#include "StandardModelProxyWidget.h"
#include "ExternalOrderImporter.h"
#include "MarketDataProvider.h"
#include "ExternalOrder.h"
#include "TaskConstants.h"
#include "PriceType.h"

class QAbstractItemView;
class QStackedWidget;
class QPushButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

namespace Evernus
{
    class AdjustableTableView;
    class EveTypeRepository;
    class EveDataProvider;

    class RegionAnalysisWidget
        : public StandardModelProxyWidget
    {
        Q_OBJECT

    public:
        RegionAnalysisWidget(const EveDataProvider &dataProvider,
                             const MarketDataProvider &marketDataProvider,
                             QWidget *parent = nullptr);
        virtual ~RegionAnalysisWidget() = default;

        void setPriceTypes(PriceType src, PriceType dst) noexcept;

        void setBogusOrderThreshold(double value) noexcept;
        void discardBogusOrders(bool flag) noexcept;

        void setCharacter(const std::shared_ptr<Character> &character);

    signals:
        void preferencesChanged();

    public slots:
        void showForCurrentRegion();

    private slots:
        void showForCurrentRegionAndSolarSystem();

        void applyRegionFilter();

        void showDetails(const QModelIndex &item);
        void selectRegionType(const QItemSelection &selected);

        void showDetailsForCurrent();

    private:
        using HistoryOrdersPair = std::pair<const MarketDataProvider::HistoryMap *, const MarketDataProvider::OrderResultType *>;

        static const auto waitingLabelIndex = 0;

        const EveDataProvider &mDataProvider;
        const MarketDataProvider &mMarketDataProvider;

        PriceType mSrcPriceType = PriceType::Buy;
        PriceType mDstPriceType = PriceType::Buy;

        QAction *mShowDetailsAct = nullptr;

        QComboBox *mRegionCombo = nullptr;
        QComboBox *mSolarSystemCombo = nullptr;
        QStackedWidget *mRegionDataStack = nullptr;
        AdjustableTableView *mRegionTypeDataView = nullptr;
        QLineEdit *mMinRegionVolumeEdit = nullptr;
        QLineEdit *mMaxRegionVolumeEdit = nullptr;
        QLineEdit *mMinRegionMarginEdit = nullptr;
        QLineEdit *mMaxRegionMarginEdit = nullptr;
        QLineEdit *mMinBuyPriceEdit = nullptr;
        QLineEdit *mMaxBuyPriceEdit = nullptr;
        QLineEdit *mMinSellPriceEdit = nullptr;
        QLineEdit *mMaxSellPriceEdit = nullptr;
        QSpinBox *mAvgDaysEdit = nullptr;
        QCheckBox *mIgnorePricePercentilesBtn = nullptr;

        MarketDataProvider::HistoryMap mEmptyHistory;
        MarketDataProvider::OrderResultType mEmptyOrders;

        TypeAggregatedMarketDataModel mTypeDataModel;
        TypeAggregatedMarketDataFilterProxyModel mTypeViewProxy;

        void fillSolarSystems(uint regionId);

        uint getCurrentRegion() const;
        HistoryOrdersPair getHistoryAndOrders(uint region);
    };
}
