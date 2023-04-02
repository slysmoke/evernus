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

#include <QWidget>

#include "ImportingDataModelProxyModel.h"
#include "StandardModelProxyWidget.h"
#include "ImportingDataModel.h"
#include "Character.h"
#include "EveType.h"

class QItemSelection;
class QStackedWidget;
class QDoubleSpinBox;
class QRadioButton;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class QString;
class QAction;

namespace Evernus
{
    class RegionStationPresetRepository;
    class AdjustableTableView;
    class MarketDataProvider;
    class EveDataProvider;

    class ImportingAnalysisWidget
        : public StandardModelProxyWidget
    {
        Q_OBJECT

    public:
        ImportingAnalysisWidget(const EveDataProvider &dataProvider,
                                const MarketDataProvider &marketDataProvider,
                                const RegionStationPresetRepository &regionStationPresetRepository,
                                QWidget *parent = nullptr);
        ImportingAnalysisWidget(const ImportingAnalysisWidget &) = default;
        ImportingAnalysisWidget(ImportingAnalysisWidget &&) = default;
        virtual ~ImportingAnalysisWidget() = default;

        void setPriceTypes(PriceType src, PriceType dst) noexcept;

        void setBogusOrderThreshold(double value) noexcept;
        void discardBogusOrders(bool flag) noexcept;

        void setCharacter(const std::shared_ptr<Character> &character);
        void recalculateData();
        void clearData();
        void completeImport();

        ImportingAnalysisWidget &operator =(const ImportingAnalysisWidget &) = default;
        ImportingAnalysisWidget &operator =(ImportingAnalysisWidget &&) = default;

    signals:
        void preferencesChanged();

    private slots:
        void showDetails(const QModelIndex &item);
        void showDetailsForCurrent();

        void selectType(const QItemSelection &selected);

    private:
        static const auto waitingLabelIndex = 0;

        const EveDataProvider &mDataProvider;
        const MarketDataProvider &mMarketDataProvider;

        QSpinBox *mAnalysisDaysEdit = nullptr;
        QSpinBox *mAggrDaysEdit = nullptr;
        QDoubleSpinBox *mPricePerM3Edit = nullptr;
        QSpinBox *mCollateralEdit = nullptr;
        QRadioButton *mCollateralBuyTypeBtn = nullptr;
        QRadioButton *mCollateralSellTypeBtn = nullptr;
        QCheckBox *mIgnoreEmptySellBtn = nullptr;
        QLineEdit *mMinAvgVolumeEdit = nullptr;
        QLineEdit *mMaxAvgVolumeEdit = nullptr;
        QLineEdit *mMinPriceDifferenceEdit = nullptr;
        QLineEdit *mMaxPriceDifferenceEdit = nullptr;
        QLineEdit *mMinMarginEdit = nullptr;
        QLineEdit *mMaxMarginEdit = nullptr;
        QStackedWidget *mDataStack = nullptr;
        AdjustableTableView *mDataView = nullptr;

        QAction *mShowDetailsAct = nullptr;

        quint64 mSrcStation = 0;
        quint64 mDstStation = 0;

        PriceType mSrcPriceType = PriceType::Buy;
        PriceType mDstPriceType = PriceType::Buy;

        ImportingDataModel mDataModel;
        ImportingDataModelProxyModel mDataProxy;

        std::shared_ptr<Character> mCharacter;

        bool mImportedNewData = true;

        void changeStation(quint64 &destination, const QVariantList &path, const QString &settingName);
    };
}
