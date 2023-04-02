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

#include <QVariant>

#include "InterRegionMarketDataFilterProxyModel.h"
#include "InterRegionMarketDataModel.h"
#include "StandardModelProxyWidget.h"
#include "ExternalOrder.h"
#include "TaskConstants.h"
#include "PriceType.h"

class QAbstractItemView;
class QStackedWidget;
class QPushButton;
class QModelIndex;
class QTableView;
class QCheckBox;
class QLineEdit;

namespace Evernus
{
    class SourceDestinationSelectWidget;
    class RegionStationPresetRepository;
    class AdjustableTableView;
    class MarketDataProvider;
    class EveDataProvider;

    class InterRegionAnalysisWidget
        : public StandardModelProxyWidget
    {
        Q_OBJECT

    public:
        InterRegionAnalysisWidget(const EveDataProvider &dataProvider,
                                  const MarketDataProvider &marketDataProvider,
                                  const RegionStationPresetRepository &regionStationPresetRepository,
                                  QWidget *parent = nullptr);
        virtual ~InterRegionAnalysisWidget() = default;

        void setPriceTypes(PriceType src, PriceType dst) noexcept;

        void setBogusOrderThreshold(double value) noexcept;
        void discardBogusOrders(bool flag) noexcept;

        void setCharacter(const std::shared_ptr<Character> &character);

        void recalculateAllData();
        void completeImport();
        void clearData();

    signals:
        void preferencesChanged();

    private slots:
        void applyInterRegionFilter();

        void showDetails(const QModelIndex &item);
        void showDetailsForCurrent();

        void selectInterRegionType(const QItemSelection &selected);

        void changeStations(const QVariantList &srcPath, const QVariantList &dstPath);

    private:
        static const auto waitingLabelIndex = 0;

        const EveDataProvider &mDataProvider;
        const MarketDataProvider &mMarketDataProvider;

        QAction *mShowDetailsAct = nullptr;

        SourceDestinationSelectWidget *mSelectWidget = nullptr;
        QLineEdit *mMinInterRegionVolumeEdit = nullptr;
        QLineEdit *mMaxInterRegionVolumeEdit = nullptr;
        QLineEdit *mMinInterRegionMarginEdit = nullptr;
        QLineEdit *mMaxInterRegionMarginEdit = nullptr;
        QStackedWidget *mInterRegionDataStack = nullptr;
        AdjustableTableView *mInterRegionTypeDataView = nullptr;

        InterRegionMarketDataModel mInterRegionDataModel;
        InterRegionMarketDataFilterProxyModel mInterRegionViewProxy;
        bool mRefreshedInterRegionData = false;

        quint64 mSrcStation = 0;
        quint64 mDstStation = 0;

        PriceType mSrcPriceType = PriceType::Buy;
        PriceType mDstPriceType = PriceType::Buy;

        void changeStation(quint64 &destination, const QVariantList &path, const QString &settingName);
        void recalculateInterRegionData();
    };
}
