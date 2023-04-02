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
#include <QtDebug>

#include <QStandardItemModel>
#include <QDoubleValidator>
#include <QStackedWidget>
#include <QIntValidator>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QMessageBox>
#include <QTableView>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <QAction>
#include <QLabel>

#include "LookupActionGroupModelConnector.h"
#include "SourceDestinationSelectWidget.h"
#include "InterRegionTypeDetailsWidget.h"
#include "MarketAnalysisSettings.h"
#include "CalculatingDataWidget.h"
#include "AdjustableTableView.h"
#include "MarketAnalysisUtils.h"
#include "MarketDataProvider.h"
#include "EveDataProvider.h"
#include "ImportSettings.h"
#include "PriceSettings.h"
#include "SSOMessageBox.h"
#include "FlowLayout.h"

#include "InterRegionAnalysisWidget.h"

namespace Evernus
{
    InterRegionAnalysisWidget::InterRegionAnalysisWidget(const EveDataProvider &dataProvider,
                                                         const MarketDataProvider &marketDataProvider,
                                                         const RegionStationPresetRepository &regionStationPresetRepository,
                                                         QWidget *parent)
        : StandardModelProxyWidget{mInterRegionDataModel, mInterRegionViewProxy, parent}
        , mDataProvider{dataProvider}
        , mMarketDataProvider{marketDataProvider}
        , mInterRegionDataModel{mDataProvider}
        , mInterRegionViewProxy{InterRegionMarketDataModel::getSrcRegionColumn(),
                                InterRegionMarketDataModel::getDstRegionColumn(),
                                InterRegionMarketDataModel::getVolumeColumn(),
                                InterRegionMarketDataModel::getMarginColumn()}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new FlowLayout{};
        mainLayout->addLayout(toolBarLayout);

        QSettings settings;

        const auto srcStationPath = settings.value(MarketAnalysisSettings::srcStationKey).toList();
        mSrcStation = EveDataProvider::getStationIdFromPath(srcStationPath);
        const auto dstStationPath = settings.value(MarketAnalysisSettings::dstStationKey).toList();
        mDstStation = EveDataProvider::getStationIdFromPath(dstStationPath);

        mSelectWidget = new SourceDestinationSelectWidget{srcStationPath,
                                                          dstStationPath,
                                                          MarketAnalysisSettings::srcRegionKey,
                                                          MarketAnalysisSettings::dstRegionKey,
                                                          mDataProvider,
                                                          regionStationPresetRepository,
                                                          this};
        toolBarLayout->addWidget(mSelectWidget);
        connect(mSelectWidget, &SourceDestinationSelectWidget::stationsChanged,
                this, &InterRegionAnalysisWidget::changeStations);

        auto volumeValidator = new QIntValidator{this};
        volumeValidator->setBottom(0);
        toolBarLayout->addWidget(new QLabel{tr("Volume:"), this});

        auto value = settings.value(MarketAnalysisSettings::minVolumeFilterKey);

        mMinInterRegionVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinInterRegionVolumeEdit);
        mMinInterRegionVolumeEdit->setValidator(volumeValidator);

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxVolumeFilterKey);

        mMaxInterRegionVolumeEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxInterRegionVolumeEdit);
        mMaxInterRegionVolumeEdit->setValidator(volumeValidator);

        auto marginValidator = new QIntValidator{this};

        toolBarLayout->addWidget(new QLabel{tr("Margin:"), this});

        value = settings.value(MarketAnalysisSettings::minMarginFilterKey);

        mMinInterRegionMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMinInterRegionMarginEdit);
        mMinInterRegionMarginEdit->setValidator(marginValidator);
        mMinInterRegionMarginEdit->setPlaceholderText(locale().percent());

        toolBarLayout->addWidget(new QLabel{"-", this});

        value = settings.value(MarketAnalysisSettings::maxMarginFilterKey);

        mMaxInterRegionMarginEdit = new QLineEdit{(value.isValid()) ? (value.toString()) : (QString{}), this};
        toolBarLayout->addWidget(mMaxInterRegionMarginEdit);
        mMaxInterRegionMarginEdit->setValidator(marginValidator);
        mMaxInterRegionMarginEdit->setPlaceholderText(locale().percent());

        auto filterBtn = new QPushButton{tr("Apply"), this};
        toolBarLayout->addWidget(filterBtn);
        connect(filterBtn, &QPushButton::clicked, this, &InterRegionAnalysisWidget::applyInterRegionFilter);

        toolBarLayout->addWidget(new QLabel{tr("Press \"Apply\" to show results. Additional actions are available via the right-click menu."), this});

        mInterRegionDataStack = new QStackedWidget{this};
        mainLayout->addWidget(mInterRegionDataStack);

        mInterRegionDataStack->addWidget(new CalculatingDataWidget{this});

        mInterRegionViewProxy.setSortRole(Qt::UserRole);
        mInterRegionViewProxy.setSourceModel(&mInterRegionDataModel);

        mInterRegionTypeDataView = new AdjustableTableView{QStringLiteral("marketAnalysisInterRegionView"), this};
        mInterRegionDataStack->addWidget(mInterRegionTypeDataView);
        mInterRegionTypeDataView->setSortingEnabled(true);
        mInterRegionTypeDataView->setAlternatingRowColors(true);
        mInterRegionTypeDataView->setModel(&mInterRegionViewProxy);
        mInterRegionTypeDataView->setContextMenuPolicy(Qt::ActionsContextMenu);
        mInterRegionTypeDataView->restoreHeaderState();
        connect(mInterRegionTypeDataView, &QTableView::doubleClicked, this, &InterRegionAnalysisWidget::showDetails);
        connect(mInterRegionTypeDataView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &InterRegionAnalysisWidget::selectInterRegionType);

        mInterRegionDataStack->setCurrentWidget(mInterRegionTypeDataView);

        mShowDetailsAct = new QAction{tr("Show details"), this};
        mShowDetailsAct->setEnabled(false);
        mInterRegionTypeDataView->addAction(mShowDetailsAct);
        connect(mShowDetailsAct, &QAction::triggered, this, &InterRegionAnalysisWidget::showDetailsForCurrent);

        new LookupActionGroupModelConnector{mInterRegionDataModel, mInterRegionViewProxy, *mInterRegionTypeDataView, this};

        installOnView(mInterRegionTypeDataView);
    }

    void InterRegionAnalysisWidget::setPriceTypes(PriceType src, PriceType dst) noexcept
    {
        mSrcPriceType = src;
        mDstPriceType = dst;
    }

    void InterRegionAnalysisWidget::setBogusOrderThreshold(double value) noexcept
    {
        mInterRegionDataModel.setBogusOrderThreshold(value);
    }

    void InterRegionAnalysisWidget::discardBogusOrders(bool flag) noexcept
    {
        mInterRegionDataModel.discardBogusOrders(flag);
    }

    void InterRegionAnalysisWidget::setCharacter(const std::shared_ptr<Character> &character)
    {
        StandardModelProxyWidget::setCharacter((character) ? (character->getId()) : (Character::invalidId));
        mInterRegionDataModel.setCharacter(character);
    }

    void InterRegionAnalysisWidget::recalculateAllData()
    {
        mRefreshedInterRegionData = false;
        applyInterRegionFilter();
    }

    void InterRegionAnalysisWidget::completeImport()
    {
        mRefreshedInterRegionData = false;
    }

    void InterRegionAnalysisWidget::clearData()
    {
        mInterRegionDataModel.reset();
    }

    void InterRegionAnalysisWidget::applyInterRegionFilter()
    {
        const auto srcRegions = mSelectWidget->getSrcSelectedRegionList();
        const auto dstRegions = mSelectWidget->getDstSelectedRegionList();

        if (!mRefreshedInterRegionData)
            recalculateInterRegionData();

        const auto minVolume = mMinInterRegionVolumeEdit->text();
        const auto maxVolume = mMaxInterRegionVolumeEdit->text();
        const auto minMargin = mMinInterRegionMarginEdit->text();
        const auto maxMargin = mMaxInterRegionMarginEdit->text();

        QSettings settings;
        settings.setValue(MarketAnalysisSettings::minVolumeFilterKey, minVolume);
        settings.setValue(MarketAnalysisSettings::maxVolumeFilterKey, maxVolume);
        settings.setValue(MarketAnalysisSettings::minMarginFilterKey, minMargin);
        settings.setValue(MarketAnalysisSettings::maxMarginFilterKey, maxMargin);

        mInterRegionViewProxy.setFilter(srcRegions,
                                        dstRegions,
                                        (minVolume.isEmpty()) ? (InterRegionMarketDataFilterProxyModel::VolumeValueType{}) : (minVolume.toUInt()),
                                        (maxVolume.isEmpty()) ? (InterRegionMarketDataFilterProxyModel::VolumeValueType{}) : (maxVolume.toUInt()),
                                        (minMargin.isEmpty()) ? (InterRegionMarketDataFilterProxyModel::MarginValueType{}) : (minMargin.toDouble()),
                                        (maxMargin.isEmpty()) ? (InterRegionMarketDataFilterProxyModel::MarginValueType{}) : (maxMargin.toDouble()));

        mInterRegionTypeDataView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

        if (!mRefreshedInterRegionData)
        {
            mInterRegionDataStack->setCurrentWidget(mInterRegionTypeDataView);
            mRefreshedInterRegionData = true;
        }
    }

    void InterRegionAnalysisWidget::showDetails(const QModelIndex &item)
    {
        const auto mappedItem = mInterRegionViewProxy.mapToSource(item);
        const auto id = mInterRegionDataModel.getTypeId(mappedItem);
        const auto srcRegion = mInterRegionDataModel.getSrcRegionId(mappedItem);
        const auto dstRegion = mInterRegionDataModel.getDstRegionId(mappedItem);

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
            connect(this, &InterRegionAnalysisWidget::preferencesChanged,
                    widget, &InterRegionTypeDetailsWidget::handleNewPreferences);
        }
        else
        {
            MarketAnalysisUtils::showMissingHistoryMessage(this);
        }
    }

    void InterRegionAnalysisWidget::showDetailsForCurrent()
    {
        showDetails(mInterRegionTypeDataView->currentIndex());
    }

    void InterRegionAnalysisWidget::selectInterRegionType(const QItemSelection &selected)
    {
        const auto enabled = !selected.isEmpty();
        mShowDetailsAct->setEnabled(enabled);
    }

    void InterRegionAnalysisWidget::changeStations(const QVariantList &srcPath, const QVariantList &dstPath)
    {
        changeStation(mSrcStation, srcPath, MarketAnalysisSettings::srcStationKey);
        changeStation(mDstStation, dstPath, MarketAnalysisSettings::dstStationKey);

        if (QMessageBox::question(this, tr("Station change"), tr("Changing station requires data recalculation. Do you wish to do it now?")) == QMessageBox::No)
            return;

        recalculateInterRegionData();
        mInterRegionDataStack->setCurrentWidget(mInterRegionTypeDataView);
    }

    void InterRegionAnalysisWidget::changeStation(quint64 &destination, const QVariantList &path, const QString &settingName)
    {
        QSettings settings;
        settings.setValue(settingName, path);

        destination = EveDataProvider::getStationIdFromPath(path);
    }

    void InterRegionAnalysisWidget::recalculateInterRegionData()
    {
        qDebug() << "Recomputing inter-region data...";

        const auto history = mMarketDataProvider.getHistory();
        if (history == nullptr)
            return;

        const auto orders = mMarketDataProvider.getOrders();
        if (orders == nullptr)
            return;

        mInterRegionDataStack->setCurrentIndex(waitingLabelIndex);
        mInterRegionDataStack->repaint();

        mInterRegionDataModel.setOrderData(*orders,
                                           *history,
                                           mSrcStation,
                                           mDstStation,
                                           mSrcPriceType,
                                           mDstPriceType);
    }
}
