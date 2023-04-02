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
#include <algorithm>
#include <limits>

#include <QDoubleValidator>
#include <QStackedWidget>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QAction>
#include <QLabel>

#include "LookupActionGroupModelConnector.h"
#include "InterRegionTypeDetailsWidget.h"
#include "FavoriteLocationsButton.h"
#include "MarketAnalysisSettings.h"
#include "CalculatingDataWidget.h"
#include "StationSelectButton.h"
#include "AdjustableTableView.h"
#include "MarketAnalysisUtils.h"
#include "MarketDataProvider.h"
#include "EveDataProvider.h"
#include "FlowLayout.h"

#include "ImportingAnalysisWidget.h"

namespace Evernus
{
    ImportingAnalysisWidget::ImportingAnalysisWidget(const EveDataProvider &dataProvider,
                                                     const MarketDataProvider &marketDataProvider,
                                                     const RegionStationPresetRepository &regionStationPresetRepository,
                                                     QWidget *parent)
        : StandardModelProxyWidget(mDataModel, mDataProxy, parent)
        , mDataProvider(dataProvider)
        , mMarketDataProvider(marketDataProvider)
        , mDataModel(mDataProvider)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new FlowLayout{};
        mainLayout->addLayout(toolBarLayout);

        toolBarLayout->addWidget(new QLabel{tr("Source:"), this});

        QSettings settings;

        const auto srcStationPath = settings.value(MarketAnalysisSettings::srcImportStationKey).toList();
        mSrcStation = EveDataProvider::getStationIdFromPath(srcStationPath);
        const auto dstStationPath = settings.value(MarketAnalysisSettings::dstImportStationKey).toList();
        mDstStation = EveDataProvider::getStationIdFromPath(dstStationPath);

        const auto srcStationBtn = new StationSelectButton{mDataProvider, srcStationPath, this};
        toolBarLayout->addWidget(srcStationBtn);
        connect(srcStationBtn, &StationSelectButton::stationChanged, this, [=](const auto &path) {
            changeStation(mSrcStation, path, MarketAnalysisSettings::srcImportStationKey);
        });

        toolBarLayout->addWidget(new QLabel{tr("Destination:"), this});

        const auto dstStationBtn = new StationSelectButton{mDataProvider, dstStationPath, this};
        toolBarLayout->addWidget(dstStationBtn);
        connect(dstStationBtn, &StationSelectButton::stationChanged, this, [=](const auto &path) {
            changeStation(mDstStation, path, MarketAnalysisSettings::dstImportStationKey);
        });

        const auto locationFavBtn = new FavoriteLocationsButton{regionStationPresetRepository, dataProvider, this};
        toolBarLayout->addWidget(locationFavBtn);
        connect(locationFavBtn, &FavoriteLocationsButton::locationsChosen,
                this, [=](const auto &, auto srcStationId, const auto &, auto dstStationId) {
            srcStationBtn->setSelectedStationId(srcStationId);
            dstStationBtn->setSelectedStationId(dstStationId);
        });

        toolBarLayout->addWidget(new QLabel{tr("Analysis period:"), this});

        mAnalysisDaysEdit = new QSpinBox{this};
        toolBarLayout->addWidget(mAnalysisDaysEdit);
        mAnalysisDaysEdit->setRange(1, 365);
        mAnalysisDaysEdit->setSuffix(tr(" days"));
        mAnalysisDaysEdit->setToolTip(tr("The number of days going back from today, to use for analysis. If the destination has been in use for shorter time, be sure to adjust this accordingly."));
        mAnalysisDaysEdit->setValue(
            settings.value(MarketAnalysisSettings::importingAnalysisDaysKey, MarketAnalysisSettings::importingAnalysisDaysDefault).toInt());

        toolBarLayout->addWidget(new QLabel{tr("Aggregate over:"), this});

        mAggrDaysEdit = new QSpinBox{this};
        toolBarLayout->addWidget(mAggrDaysEdit);
        mAggrDaysEdit->setRange(1, 365);
        mAggrDaysEdit->setSuffix(tr(" days"));
        mAggrDaysEdit->setToolTip(tr("The number of days to aggregate movement over. This should reflect how fast you want your stock to sell."));
        mAggrDaysEdit->setValue(
            settings.value(MarketAnalysisSettings::importingAggrDaysKey, MarketAnalysisSettings::importingAggrDaysDefault).toInt());

        toolBarLayout->addWidget(new QLabel{tr("Price per m³:"), this});

        mPricePerM3Edit = new QDoubleSpinBox{this};
        toolBarLayout->addWidget(mPricePerM3Edit);
        mPricePerM3Edit->setMaximum(std::numeric_limits<double>::max());
        mPricePerM3Edit->setSuffix(QStringLiteral("ISK"));
        mPricePerM3Edit->setToolTip(tr("Addtional cost added to buy price. This is multiplied by item size and desired volume to move (which in turn is based on aggregation days)."));
        mPricePerM3Edit->setValue(settings.value(MarketAnalysisSettings::importingPricePerM3Key).toDouble());

        toolBarLayout->addWidget(new QLabel{tr("Collateral:"), this});

        mCollateralEdit = new QSpinBox{this};
        toolBarLayout->addWidget(mCollateralEdit);
        mCollateralEdit->setRange(0, 100);
        mCollateralEdit->setSuffix(locale().percent());
        mCollateralEdit->setToolTip(tr("Additional cost added to buy price. This is the percentage of the base price."));
        mCollateralEdit->setValue(settings.value(MarketAnalysisSettings::importingCollateralKey).toInt());

        const auto collateralType = static_cast<PriceType>(
            settings.value(MarketAnalysisSettings::importingCollateralPriceTypeKey, MarketAnalysisSettings::importingCollateralPriceTypeDefault).toInt());

        mCollateralBuyTypeBtn = new QRadioButton{tr("buy"), this};
        toolBarLayout->addWidget(mCollateralBuyTypeBtn);
        mCollateralBuyTypeBtn->setChecked(collateralType != PriceType::Sell);

        mCollateralSellTypeBtn = new QRadioButton{tr("sell"), this};
        toolBarLayout->addWidget(mCollateralSellTypeBtn);
        mCollateralSellTypeBtn->setChecked(collateralType == PriceType::Sell);

        mIgnoreEmptySellBtn = new QCheckBox{tr("Hide empty source sell orders"), this};
        toolBarLayout->addWidget(mIgnoreEmptySellBtn);
        mIgnoreEmptySellBtn->setToolTip(tr("Hide item types which have 0 source orders when source price type is set to \"Sell\"."));
        mIgnoreEmptySellBtn->setChecked(
            settings.value(MarketAnalysisSettings::importingHideEmptySellOrdersKey, MarketAnalysisSettings::importingHideEmptySellOrdersDefault).toBool());

        const auto filterValueValidator = new QDoubleValidator{this};
        filterValueValidator->setBottom(0.);

        toolBarLayout->addWidget(new QLabel{tr("Avg. volume:"), this});

        auto value = settings.value(MarketAnalysisSettings::minAvgVolumeFilterKey);

        mMinAvgVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinAvgVolumeEdit);
        mMinAvgVolumeEdit->setValidator(filterValueValidator);

        toolBarLayout->addWidget(new QLabel{QStringLiteral("-"), this});

        value = settings.value(MarketAnalysisSettings::maxAvgVolumeFilterKey);

        mMaxAvgVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxAvgVolumeEdit);
        mMaxAvgVolumeEdit->setValidator(filterValueValidator);

        toolBarLayout->addWidget(new QLabel{tr("Price diff.:"), this});

        value = settings.value(MarketAnalysisSettings::minPriceDifferenceFilterKey);

        mMinPriceDifferenceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinPriceDifferenceEdit);
        mMinPriceDifferenceEdit->setValidator(filterValueValidator);

        toolBarLayout->addWidget(new QLabel{QStringLiteral("-"), this});

        value = settings.value(MarketAnalysisSettings::maxPriceDifferenceFilterKey);

        mMaxPriceDifferenceEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxPriceDifferenceEdit);
        mMaxPriceDifferenceEdit->setValidator(filterValueValidator);

        toolBarLayout->addWidget(new QLabel{tr("Margin:"), this});

        value = settings.value(MarketAnalysisSettings::minMarginFilterKey);

        mMinMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinMarginEdit);
        mMinMarginEdit->setValidator(filterValueValidator);

        toolBarLayout->addWidget(new QLabel{QStringLiteral("-"), this});

        value = settings.value(MarketAnalysisSettings::maxMarginFilterKey);

        mMaxMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxMarginEdit);
        mMaxMarginEdit->setValidator(filterValueValidator);

        auto filterBtn = new QPushButton{tr("Apply"), this};
        toolBarLayout->addWidget(filterBtn);
        connect(filterBtn, &QPushButton::clicked, this, [=] {
            mImportedNewData = true;
            recalculateData();
        });

        toolBarLayout->addWidget(new QLabel{tr("Press \"Apply\" to show results. Additional actions are available via the right-click menu."), this});

        mDataStack = new QStackedWidget{this};
        mainLayout->addWidget(mDataStack);

        mDataStack->addWidget(new CalculatingDataWidget{this});

        mDataProxy.setSortRole(Qt::UserRole);
        mDataProxy.setFilterRole(Qt::UserRole);
        mDataProxy.setSourceModel(&mDataModel);

        mDataView = new AdjustableTableView{QStringLiteral("marketAnalysisImportingView"), this};
        mDataStack->addWidget(mDataView);
        mDataView->setSortingEnabled(true);
        mDataView->setAlternatingRowColors(true);
        mDataView->setModel(&mDataProxy);
        mDataView->setContextMenuPolicy(Qt::ActionsContextMenu);
        mDataView->restoreHeaderState();
        connect(mDataView, &QTableView::doubleClicked, this, &ImportingAnalysisWidget::showDetails);
        connect(mDataView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &ImportingAnalysisWidget::selectType);

        mDataStack->setCurrentWidget(mDataView);

        mShowDetailsAct = new QAction{tr("Show details"), this};
        mShowDetailsAct->setEnabled(false);
        mDataView->addAction(mShowDetailsAct);
        connect(mShowDetailsAct, &QAction::triggered, this, &ImportingAnalysisWidget::showDetailsForCurrent);

        new LookupActionGroupModelConnector{mDataModel, mDataProxy, *mDataView, this};

        installOnView(mDataView);
    }

    void ImportingAnalysisWidget::setPriceTypes(PriceType src, PriceType dst) noexcept
    {
        mSrcPriceType = src;
        mDstPriceType = dst;
    }

    void ImportingAnalysisWidget::setBogusOrderThreshold(double value) noexcept
    {
        mDataModel.setBogusOrderThreshold(value);
    }

    void ImportingAnalysisWidget::discardBogusOrders(bool flag) noexcept
    {
        mDataModel.discardBogusOrders(flag);
    }

    void ImportingAnalysisWidget::setCharacter(const std::shared_ptr<Character> &character)
    {
        mCharacter = character;
        mDataModel.setCharacter(mCharacter);

        StandardModelProxyWidget::setCharacter((mCharacter) ? (mCharacter->getId()) : (Character::invalidId));
    }

    void ImportingAnalysisWidget::recalculateData()
    {
        if (mSrcStation == 0 || mDstStation == 0)
            return;

        qDebug() << "Recomputing importing data...";

        mDataStack->setCurrentIndex(waitingLabelIndex);
        mDataStack->repaint();

        const auto history = mMarketDataProvider.getHistory();
        if (history == nullptr)
            return;

        const auto orders = mMarketDataProvider.getOrders();
        if (orders == nullptr)
            return;

        const auto analysisDays = mAnalysisDaysEdit->value();
        const auto aggrDays = mAggrDaysEdit->value();
        const auto pricePerM3 = mPricePerM3Edit->value();
        const auto collateral = mCollateralEdit->value() / 100.;
        const auto collateralPriceType = (mCollateralBuyTypeBtn->isChecked()) ? (PriceType::Buy) : (PriceType::Sell);
        const auto hideEmptySell = mIgnoreEmptySellBtn->isChecked();
        const auto minAvgVolume = mMinAvgVolumeEdit->text();
        const auto maxAvgVolume = mMaxAvgVolumeEdit->text();
        const auto minPriceDifference = mMinPriceDifferenceEdit->text();
        const auto maxPriceDifference = mMaxPriceDifferenceEdit->text();
        const auto minMargin = mMinMarginEdit->text();
        const auto maxMargin = mMaxMarginEdit->text();

        QSettings settings;
        settings.setValue(MarketAnalysisSettings::importingAnalysisDaysKey, analysisDays);
        settings.setValue(MarketAnalysisSettings::importingAggrDaysKey, aggrDays);
        settings.setValue(MarketAnalysisSettings::importingPricePerM3Key, pricePerM3);
        settings.setValue(MarketAnalysisSettings::importingCollateralKey, collateral);
        settings.setValue(MarketAnalysisSettings::importingCollateralPriceTypeKey, static_cast<int>(collateralPriceType));
        settings.setValue(MarketAnalysisSettings::importingHideEmptySellOrdersKey, hideEmptySell);
        settings.setValue(MarketAnalysisSettings::minAvgVolumeFilterKey, minAvgVolume);
        settings.setValue(MarketAnalysisSettings::maxAvgVolumeFilterKey, maxAvgVolume);
        settings.setValue(MarketAnalysisSettings::minPriceDifferenceFilterKey, minPriceDifference);
        settings.setValue(MarketAnalysisSettings::maxPriceDifferenceFilterKey, maxPriceDifference);
        settings.setValue(MarketAnalysisSettings::minMarginFilterKey, minMargin);
        settings.setValue(MarketAnalysisSettings::maxMarginFilterKey, maxMargin);

        const auto optionalValue = [](const auto &value) {
            return (value.isEmpty()) ? (std::nullopt) : (std::make_optional(value.toDouble()));
        };

        mDataProxy.setFilters(optionalValue(minAvgVolume),
                              optionalValue(maxAvgVolume),
                              optionalValue(minPriceDifference),
                              optionalValue(maxPriceDifference),
                              optionalValue(minMargin),
                              optionalValue(maxMargin));

        if (mImportedNewData)
        {
            mDataModel.setOrderData(*orders,
                                    *history,
                                    mSrcStation,
                                    mDstStation,
                                    mSrcPriceType,
                                    mDstPriceType,
                                    analysisDays,
                                    std::min(analysisDays, aggrDays),
                                    pricePerM3,
                                    collateral,
                                    collateralPriceType,
                                    hideEmptySell);

            mDataView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
            mImportedNewData = false;
        }

        mDataStack->setCurrentWidget(mDataView);
    }

    void ImportingAnalysisWidget::clearData()
    {
        mDataModel.reset();
    }

    void ImportingAnalysisWidget::completeImport()
    {
        mImportedNewData = true;
    }

    void ImportingAnalysisWidget::showDetails(const QModelIndex &item)
    {
        const auto mappedItem = mDataProxy.mapToSource(item);
        const auto id = mDataModel.getTypeId(mappedItem);
        const auto srcRegion = mDataProvider.getStationRegionId(mSrcStation);
        const auto dstRegion = mDataProvider.getStationRegionId(mDstStation);

        const auto srcHistory = mMarketDataProvider.getHistory(srcRegion);
        if (srcHistory == nullptr)
            return;

        const auto dstHistory = mMarketDataProvider.getHistory(dstRegion);
        if (dstHistory == nullptr)
            return;

        const auto srcIt = srcHistory->find(id);
        const auto dstIt = dstHistory->find(id);
        if (srcIt != std::end(*srcHistory) && dstIt != std::end(*dstHistory))
        {
            auto widget = new InterRegionTypeDetailsWidget{srcIt->second,
                                                           dstIt->second,
                                                           mDataProvider.getRegionName(srcRegion),
                                                           mDataProvider.getRegionName(dstRegion),
                                                           this,
                                                           Qt::Window};
            widget->setWindowTitle(mDataProvider.getTypeName(id));
            widget->show();
            connect(this, &ImportingAnalysisWidget::preferencesChanged,
                    widget, &InterRegionTypeDetailsWidget::handleNewPreferences);
        }
        else
        {
            MarketAnalysisUtils::showMissingHistoryMessage(this);
        }
    }

    void ImportingAnalysisWidget::showDetailsForCurrent()
    {
        showDetails(mDataView->currentIndex());
    }

    void ImportingAnalysisWidget::selectType(const QItemSelection &selected)
    {
        const auto enabled = !selected.isEmpty();
        mShowDetailsAct->setEnabled(enabled);
    }

    void ImportingAnalysisWidget::changeStation(quint64 &destination, const QVariantList &path, const QString &settingName)
    {
        QSettings settings;
        settings.setValue(settingName, path);

        destination = EveDataProvider::getStationIdFromPath(path);

        if (destination != 0)
        {
            if (QMessageBox::question(this, tr("Station change"), tr("Changing station requires data recalculation. Do you wish to do it now?")) == QMessageBox::Yes)
            {
                mImportedNewData = true;
                recalculateData();
            }
        }
    }
}
