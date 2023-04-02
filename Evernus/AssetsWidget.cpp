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
#include <limits>

#include <QSortFilterProxyModel>
#include <QInputDialog>
#include <QRadioButton>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QCheckBox>
#include <QGroupBox>
#include <QSettings>
#include <QAction>
#include <QLocale>
#include <QtDebug>
#include <QLabel>

#include "AdjustableTableView.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "TextFilterWidget.h"
#include "ButtonWithTimer.h"
#include "StyledTreeView.h"
#include "ImportSettings.h"
#include "AssetProvider.h"
#include "StationView.h"
#include "UISettings.h"
#include "AssetList.h"
#include "TextUtils.h"

#include "AssetsWidget.h"

namespace Evernus
{
    AssetsWidget::AssetsWidget(AssetProvider &assetProvider,
                               const EveDataProvider &dataProvider,
                               const CacheTimerProvider &cacheTimerProvider,
                               const FilterTextRepository &filterRepo,
                               bool corp,
                               QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpAssetList) : (TimerType::AssetList)),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpAssetList) : (TimerType::AssetList)),
                               ImportSettings::maxAssetListAgeKey,
                               parent)
        , mAssetProvider(assetProvider)
        , mInventoryModel(mAssetProvider, dataProvider, !corp)
        , mAggregatedModel(mAssetProvider, dataProvider)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        auto importFromWeb = new QPushButton{QIcon{":/images/world.png"}, tr("Import prices from Web"), this};
        toolBarLayout->addWidget(importFromWeb);
        importFromWeb->setFlat(true);
        connect(importFromWeb, &QPushButton::clicked, this, &AssetsWidget::prepareItemImportFromWeb);

        auto importFromFile = new QPushButton{QIcon{":/images/page_refresh.png"}, tr("Import prices from logs"), this};
        toolBarLayout->addWidget(importFromFile);
        importFromFile->setFlat(true);
        connect(importFromFile, &QPushButton::clicked, this, &AssetsWidget::prepareItemImportFromFile);

        auto expandAll = new QPushButton{QIcon{":/images/arrow_out.png"}, tr("Expand all"), this};
        toolBarLayout->addWidget(expandAll);
        expandAll->setFlat(true);

        auto collapseAll = new QPushButton{QIcon{":/images/arrow_in.png"}, tr("Collapse all"), this};
        toolBarLayout->addWidget(collapseAll);
        collapseAll->setFlat(true);

        auto filterEdit = new TextFilterWidget{filterRepo, this};
        toolBarLayout->addWidget(filterEdit, 1);
        connect(filterEdit, &TextFilterWidget::filterEntered, this, &AssetsWidget::applyWildcard);

        QSettings settings;

        auto combineBtn = new QCheckBox{tr("Combine for all characters"), this};
        toolBarLayout->addWidget(combineBtn);
        combineBtn->setChecked(settings.value(UISettings::combineAssetsKey, UISettings::combineAssetsDefault).toBool());
        connect(combineBtn, &QCheckBox::toggled, this, [=](bool checked) {
            QSettings settings;
            settings.setValue(UISettings::combineAssetsKey, checked);

            mInventoryModel.setCombineCharacters(checked);
            mAggregatedModel.setCombineCharacters(checked);
            resetModel();
        });

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        auto assetLayout = new QHBoxLayout{};
        mainLayout->addLayout(assetLayout);

        const auto mainTabs = new QTabWidget{this};
        assetLayout->addWidget(mainTabs, 1);

        mInventoryModel.setCombineCharacters(combineBtn->isChecked());
        mInventoryModelProxy.setSourceModel(&mInventoryModel);

        mAggregatedModel.setCombineCharacters(combineBtn->isChecked());
        mAggregatedModelProxy.setSourceModel(&mAggregatedModel);

        const auto configureProxy = [=](auto &proxy) {
            proxy.setSortRole(Qt::UserRole);
            proxy.setFilterCaseSensitivity(Qt::CaseInsensitive);
        };

        configureProxy(mInventoryModelProxy);
        configureProxy(mAggregatedModelProxy);

        mAssetView = new StyledTreeView{QStringLiteral("assetView"), this};
        mainTabs->addTab(mAssetView, tr("Inventory"));
        mAssetView->setModel(&mInventoryModelProxy);
        connect(mAssetView->selectionModel(), &QItemSelectionModel::selectionChanged,
                this, &AssetsWidget::handleSelection);
        connect(expandAll, &QPushButton::clicked, mAssetView, &StyledTreeView::expandAll);
        connect(collapseAll, &QPushButton::clicked, mAssetView, &StyledTreeView::collapseAll);

        mSetDestinationAct = new QAction{tr("Set destination in EVE"), this};
        mSetDestinationAct->setEnabled(false);
        mAssetView->addAction(mSetDestinationAct);
        connect(mSetDestinationAct, &QAction::triggered, this, &AssetsWidget::setDestinationForCurrent);

        mShowInEveAct = new QAction{tr("Show in EVE"), this};
        mShowInEveAct->setEnabled(false);
        mAssetView->addAction(mShowInEveAct);
        connect(mShowInEveAct, &QAction::triggered, this, &AssetsWidget::showInEveForCurrent);

        mSetCustomValueAct = new QAction{tr("Set custom value"), this};
        mSetCustomValueAct->setEnabled(false);
        mAssetView->addAction(mSetCustomValueAct);
        connect(mSetCustomValueAct, &QAction::triggered, this, &AssetsWidget::setCustomValue);

        mClearCustomValueAct = new QAction{tr("Clear custom value"), this};
        mClearCustomValueAct->setEnabled(false);
        mAssetView->addAction(mClearCustomValueAct);
        connect(mClearCustomValueAct, &QAction::triggered, this, &AssetsWidget::clearCustomValue);

        mAggregatedView = new AdjustableTableView{"assetsWidgetAggregatedView", this};
        mainTabs->addTab(mAggregatedView, tr("Aggregated"));
        mAggregatedView->setSortingEnabled(true);
        mAggregatedView->setAlternatingRowColors(true);
        mAggregatedView->setModel(&mAggregatedModelProxy);
        mAggregatedView->restoreHeaderState();

        auto stationGroup = new QGroupBox{tr("Price station"), this};
        assetLayout->addWidget(stationGroup);

        auto stationGroupLayout = new QVBoxLayout{stationGroup};

        const auto useCustomStation
            = settings.value(ImportSettings::useCustomAssetStationKey, ImportSettings::useCustomAssetStationDefault).toBool();

        mUseAssetStationBtn = new QRadioButton{tr("Use asset location"), this};
        stationGroupLayout->addWidget(mUseAssetStationBtn);
        mUseAssetStationBtn->setChecked(!useCustomStation);
        connect(mUseAssetStationBtn, &QRadioButton::toggled, this, [=](bool checked) {
            if (checked)
                setCustomStation(0);
            else
                setCustomStation(mStationView->getStationId());

            QSettings settings;
            settings.setValue(ImportSettings::useCustomAssetStationKey, !checked);
        });

        auto customStationBtn = new QRadioButton{tr("Use custom station"), this};
        stationGroupLayout->addWidget(customStationBtn);
        customStationBtn->setChecked(useCustomStation);

        mStationView = new StationView{dataProvider, this};
        stationGroupLayout->addWidget(mStationView);
        mStationView->setEnabled(useCustomStation);
        mStationView->setMaximumWidth(260);
        mStationView->selectPath(settings.value(ImportSettings::customAssetStationKey).toList());
        connect(mStationView, &StationView::stationChanged, this, &AssetsWidget::setCustomStation);
        connect(customStationBtn, &QRadioButton::toggled, mStationView, &StationView::setEnabled);

        mInfoLabel = new QLabel{this};
        mainLayout->addWidget(mInfoLabel);

        if (useCustomStation)
        {
            mInventoryModel.setCustomStation(mStationView->getStationId());
            mAggregatedModel.setCustomStation(mStationView->getStationId());
        }
    }

    void AssetsWidget::updateData()
    {
        refreshImportTimer();
        mInventoryModel.reset();
        mAggregatedModel.reset();
        mAssetView->expandAll();

        setNewInfo();
    }

    void AssetsWidget::prepareItemImportFromWeb()
    {
        emit importPricesFromWeb(getCharacterId(), getImportTarget());
    }

    void AssetsWidget::prepareItemImportFromFile()
    {
        emit importPricesFromFile(getCharacterId(), getImportTarget());
    }

    void AssetsWidget::applyWildcard(const QString &text)
    {
        mInventoryModelProxy.setFilterWildcard(text);
        mAggregatedModelProxy.setFilterWildcard(text);
        mAssetView->expandAll();
    }

    void AssetsWidget::setCustomStation(quint64 id)
    {
        mCustomStationId = id;

        if (mCustomStationId != 0)
        {
            QSettings settings;
            settings.setValue(ImportSettings::customAssetStationKey, mStationView->getSelectedPath());
        }
        else if (!mUseAssetStationBtn->isChecked())
        {
            return;
        }

        mInventoryModel.setCustomStation(mCustomStationId);
        mInventoryModel.reset();

        mAggregatedModel.setCustomStation(mCustomStationId);
        mAggregatedModel.reset();

        mAssetView->expandAll();

        setNewInfo();
    }

    void AssetsWidget::setCustomValue()
    {
        const auto index = getCurrentIndex();
        const auto id = mInventoryModel.getAssetId(index);
        if (id == Item::invalidId)
            return;

        const auto currentValue = mInventoryModel.getAssetCustomValue(index);
        auto ok = false;

        const auto value = QInputDialog::getDouble(this, tr("Set custom value"), tr("ISK:"), (currentValue) ? (*currentValue) : (0.), 0., std::numeric_limits<double>::max(), 2, &ok);
        if (ok)
        {
            mAssetProvider.setCustomValue(id, value);
            resetModel();
        }
    }

    void AssetsWidget::clearCustomValue()
    {
        const auto index = getCurrentIndex();
        const auto id = mInventoryModel.getAssetId(index);
        if (id == Item::invalidId)
            return;

        mAssetProvider.clearCustomValue(id);
        resetModel();
    }

    void AssetsWidget::setDestinationForCurrent()
    {
        const auto id = mInventoryModel.getAssetLocationId(getCurrentIndex());
        emit setDestinationInEve(id);
    }

    void AssetsWidget::showInEveForCurrent()
    {
        const auto index = getCurrentIndex();
        emit showInEve(mInventoryModel.getAssetTypeId(index), mInventoryModel.getAssetOwnerId(index));
    }

    void AssetsWidget::handleSelection(const QItemSelection &selected)
    {
        const auto enable = !selected.isEmpty();

        mSetDestinationAct->setEnabled(enable && mInventoryModel.getAssetLocationId(getCurrentIndex()) != 0);
        mShowInEveAct->setEnabled(enable && mInventoryModel.getAssetTypeId(getCurrentIndex()) != 0);
        mSetCustomValueAct->setEnabled(enable);
        mClearCustomValueAct->setEnabled(enable);
    }

    void AssetsWidget::setCombine(int state)
    {
        QSettings settings;
    }

    void AssetsWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching assets to" << id;

        mInventoryModel.setCharacter(id);
        mAggregatedModel.setCharacter(id);

        resetModel();
    }

    void AssetsWidget::resetModel()
    {
        mInventoryModel.reset();
        mAggregatedModel.reset();

        mAssetView->expandAll();
        mAssetView->header()->resizeSections(QHeaderView::ResizeToContents);
        mAssetView->sortByColumn(0, Qt::AscendingOrder);

        mAggregatedView->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

        setNewInfo();
    }

    void AssetsWidget::setNewInfo()
    {
        QLocale locale;
        mInfoLabel->setText(QString{"Total assets: <strong>%1</strong> Total volume: <strong>%2m³</strong> Total sell price: <strong>%3</strong>"}
            .arg(locale.toString(mInventoryModel.getTotalAssets()))
            .arg(locale.toString(mInventoryModel.getTotalVolume(), 'f', 2))
            .arg(TextUtils::currencyToString(mInventoryModel.getTotalSellPrice(), locale)));
    }

    TypeLocationPairs AssetsWidget::getImportTarget() const
    {
        TypeLocationPairs target;

        auto buildImportTargetFromList = [&target, this](const auto &assets) {
            for (const auto &item : *assets)
            {
                if (mCustomStationId == 0)
                {
                    const auto locationId = item->getLocationId();
                    if (!locationId)
                        continue;

                    buildImportTarget(target, *item, *locationId);
                }
                else
                {
                    buildImportTarget(target, *item, mCustomStationId);
                }
            }
        };

        if (mInventoryModel.isCombiningCharacters())
        {
            const auto list = mAssetProvider.fetchAllAssets();
            for (const auto &assets : list)
                buildImportTargetFromList(assets);
        }
        else
        {
            buildImportTargetFromList(mAssetProvider.fetchAssetsForCharacter(getCharacterId()));
        }

        return target;
    }

    QModelIndex AssetsWidget::getCurrentIndex() const
    {
        return mInventoryModelProxy.mapToSource(mAssetView->currentIndex());
    }

    void AssetsWidget::buildImportTarget(TypeLocationPairs &target, const Item &item, quint64 locationId)
    {
        target.insert(std::make_pair(item.getTypeId(), locationId));
        for (const auto &child : item)
            buildImportTarget(target, *child, locationId);
    }
}
