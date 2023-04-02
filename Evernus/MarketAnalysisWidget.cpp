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
#include <QDoubleValidator>
#include <QVBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSettings>
#include <QLabel>
#include <QtDebug>

#include "ScrapmetalReprocessingArbitrageWidget.h"
#include "OreReprocessingArbitrageWidget.h"
#include "DontSaveImportedOrdersCheckBox.h"
#include "InterRegionAnalysisWidget.h"
#include "ImportingAnalysisWidget.h"
#include "RegionTypeSelectDialog.h"
#include "MarketAnalysisSettings.h"
#include "MarketOrderRepository.h"
#include "RegionAnalysisWidget.h"
#include "CharacterRepository.h"
#include "PriceTypeComboBox.h"
#include "EveDataProvider.h"
#include "SSOMessageBox.h"
#include "TaskManager.h"
#include "FlowLayout.h"

#include "MarketAnalysisWidget.h"

namespace Evernus
{
    MarketAnalysisWidget::MarketAnalysisWidget(const EveDataProvider &dataProvider,
                                               ESIInterfaceManager &interfaceManager,
                                               TaskManager &taskManager,
                                               const MarketOrderRepository &orderRepo,
                                               const MarketOrderRepository &corpOrderRepo,
                                               const EveTypeRepository &typeRepo,
                                               const MarketGroupRepository &groupRepo,
                                               const CharacterRepository &characterRepo,
                                               const RegionTypePresetRepository &regionTypePresetRepo,
                                               const RegionStationPresetRepository &regionStationPresetRepository,
                                               QWidget *parent)
        : QWidget{parent}
        , MarketDataProvider{}
        , mDataProvider{dataProvider}
        , mTaskManager{taskManager}
        , mOrderRepo{orderRepo}
        , mCorpOrderRepo{corpOrderRepo}
        , mTypeRepo{typeRepo}
        , mGroupRepo{groupRepo}
        , mCharacterRepo{characterRepo}
        , mRegionTypePresetRepo{regionTypePresetRepo}
        , mOrders{std::make_shared<MarketAnalysisDataFetcher::OrderResultType::element_type>()}
        , mHistory{std::make_shared<MarketAnalysisDataFetcher::HistoryResultType::element_type>()}
        , mDataFetcher{mDataProvider, interfaceManager}
    {
        connect(&mDataFetcher, &MarketAnalysisDataFetcher::orderStatusUpdated,
                this, &MarketAnalysisWidget::updateOrderTask);
        connect(&mDataFetcher, &MarketAnalysisDataFetcher::historyStatusUpdated,
                this, &MarketAnalysisWidget::updateHistoryTask);
        connect(&mDataFetcher, &MarketAnalysisDataFetcher::orderImportEnded,
                this, &MarketAnalysisWidget::endOrderTask);
        connect(&mDataFetcher, &MarketAnalysisDataFetcher::historyImportEnded,
                this, &MarketAnalysisWidget::endHistoryTask);
        connect(&mDataFetcher, &MarketAnalysisDataFetcher::genericError,
                this, [=](const auto &text) {
            SSOMessageBox::showMessage(text, this);
        });

        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new FlowLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto importFromWeb = new QPushButton{QIcon{":/images/world.png"}, tr("Import data"), this};
        toolBarLayout->addWidget(importFromWeb);
        importFromWeb->setFlat(true);
        connect(importFromWeb, &QPushButton::clicked, this, &MarketAnalysisWidget::prepareOrderImport);

        QSettings settings;

        mDontSaveBtn = new DontSaveImportedOrdersCheckBox{this};
        toolBarLayout->addWidget(mDontSaveBtn);
        mDontSaveBtn->setChecked(
            settings.value(MarketAnalysisSettings::dontSaveLargeOrdersKey, MarketAnalysisSettings::dontSaveLargeOrdersDefault).toBool());
        connect(mDontSaveBtn, &QCheckBox::toggled, [](auto checked) {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::dontSaveLargeOrdersKey, checked);
        });

        mIgnoreExistingOrdersBtn = new QCheckBox{tr("Ignore types with existing orders"), this};
        toolBarLayout->addWidget(mIgnoreExistingOrdersBtn);
        mIgnoreExistingOrdersBtn->setToolTip(tr("Ignore item types which have active market orders."));
        mIgnoreExistingOrdersBtn->setChecked(
            settings.value(MarketAnalysisSettings::ignoreExistingOrdersKey, MarketAnalysisSettings::ignoreExistingOrdersDefault).toBool());
        connect(mIgnoreExistingOrdersBtn, &QCheckBox::toggled, [](auto checked) {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::ignoreExistingOrdersKey, checked);
        });

        const auto discardBogusOrders = settings.value(MarketAnalysisSettings::discardBogusOrdersKey, MarketAnalysisSettings::discardBogusOrdersDefault).toBool();

        auto discardBogusOrdersBtn = new QCheckBox{tr("Discard bogus orders (causes recalculation)"), this};
        toolBarLayout->addWidget(discardBogusOrdersBtn);
        discardBogusOrdersBtn->setChecked(discardBogusOrders);
        discardBogusOrdersBtn->setToolTip(tr("Use heuristics based on average price to determine if a given order is legit."));
        connect(discardBogusOrdersBtn, &QCheckBox::toggled, this, [=](bool checked) {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::discardBogusOrdersKey, checked);

            mRegionAnalysisWidget->discardBogusOrders(checked);
            mInterRegionAnalysisWidget->discardBogusOrders(checked);
            mImportingAnalysisWidget->discardBogusOrders(checked);

            recalculateAllData();
        });

        toolBarLayout->addWidget(new QLabel{tr("Bogus order threshold:"), this});

        auto thresholdValidator = new QDoubleValidator{this};
        thresholdValidator->setBottom(0.);

        const auto bogusThreshold
            = settings.value(MarketAnalysisSettings::bogusOrderThresholdKey, MarketAnalysisSettings::bogusOrderThresholdDefault).toString();
        const auto bogusThresholdValue = bogusThreshold.toDouble();

        auto bogusThresholdEdit = new QLineEdit{bogusThreshold, this};
        toolBarLayout->addWidget(bogusThresholdEdit);
        bogusThresholdEdit->setValidator(thresholdValidator);
        bogusThresholdEdit->setToolTip(tr("How much a price can deviate from the average price to consider it legit."));
        connect(bogusThresholdEdit, &QLineEdit::textEdited, this, [this](const auto &text) {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::bogusOrderThresholdKey, text);

            const auto value = text.toDouble();
            mRegionAnalysisWidget->setBogusOrderThreshold(value);
            mInterRegionAnalysisWidget->setBogusOrderThreshold(value);
            mImportingAnalysisWidget->setBogusOrderThreshold(value);
        });

        auto createPriceTypeCombo = [=](auto &combo) {
            combo = new PriceTypeComboBox{this};
            toolBarLayout->addWidget(combo);

            connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=] {
                const auto src = mSrcPriceTypeCombo->getPriceType();
                const auto dst = mDstPriceTypeCombo->getPriceType();

                mRegionAnalysisWidget->setPriceTypes(src, dst);
                mInterRegionAnalysisWidget->setPriceTypes(src, dst);
                mImportingAnalysisWidget->setPriceTypes(src, dst);
                mOreReprocessingArbitrageWidget->setPriceType(dst);
                mScrapmetalReprocessingArbitrageWidget->setPriceType(dst);
            });
        };

        toolBarLayout->addWidget(new QLabel{tr("Source price:"), this});
        createPriceTypeCombo(mSrcPriceTypeCombo);
        mSrcPriceTypeCombo->blockSignals(true);
        mSrcPriceTypeCombo->setCurrentIndex(1);
        mSrcPriceTypeCombo->blockSignals(false);
        mSrcPriceTypeCombo->setToolTip(tr("Type of orders used for buying items."));

        toolBarLayout->addWidget(new QLabel{tr("Destination price:"), this});
        createPriceTypeCombo(mDstPriceTypeCombo);
        mDstPriceTypeCombo->setToolTip(tr("Type of orders used for selling items."));

        auto skillsDiffBtn = new QCheckBox{tr("Use skills and taxes for difference calculation (causes recalculation)"), this};
        toolBarLayout->addWidget(skillsDiffBtn);
        skillsDiffBtn->setChecked(
            settings.value(MarketAnalysisSettings::useSkillsForDifferenceKey, MarketAnalysisSettings::useSkillsForDifferenceDefault).toBool());
        connect(skillsDiffBtn, &QCheckBox::toggled, [=](auto checked) {
            QSettings settings;
            settings.setValue(MarketAnalysisSettings::useSkillsForDifferenceKey, checked);

            recalculateAllData();
        });

        auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs);

        const auto src = mSrcPriceTypeCombo->getPriceType();
        const auto dst = mDstPriceTypeCombo->getPriceType();

        mRegionAnalysisWidget = new RegionAnalysisWidget{mDataProvider, *this, tabs};
        connect(mRegionAnalysisWidget, &RegionAnalysisWidget::showInEve, this, &MarketAnalysisWidget::showInEve);
        mRegionAnalysisWidget->setPriceTypes(src, dst);
        mRegionAnalysisWidget->setBogusOrderThreshold(bogusThresholdValue);
        mRegionAnalysisWidget->discardBogusOrders(discardBogusOrders);

        mInterRegionAnalysisWidget = new InterRegionAnalysisWidget{mDataProvider,
                                                                   *this,
                                                                   regionStationPresetRepository,
                                                                   tabs};
        connect(mInterRegionAnalysisWidget, &InterRegionAnalysisWidget::showInEve,
                this, &MarketAnalysisWidget::showInEve);
        connect(this, &MarketAnalysisWidget::preferencesChanged,
                mInterRegionAnalysisWidget, &InterRegionAnalysisWidget::preferencesChanged);
        mInterRegionAnalysisWidget->setPriceTypes(src, dst);
        mInterRegionAnalysisWidget->setBogusOrderThreshold(bogusThresholdValue);
        mInterRegionAnalysisWidget->discardBogusOrders(discardBogusOrders);

        mImportingAnalysisWidget = new ImportingAnalysisWidget{mDataProvider, *this, regionStationPresetRepository, tabs};
        connect(mImportingAnalysisWidget, &ImportingAnalysisWidget::showInEve, this, &MarketAnalysisWidget::showInEve);
        connect(this, &MarketAnalysisWidget::preferencesChanged,
                mImportingAnalysisWidget, &ImportingAnalysisWidget::preferencesChanged);
        mImportingAnalysisWidget->setPriceTypes(src, dst);
        mImportingAnalysisWidget->setBogusOrderThreshold(bogusThresholdValue);
        mImportingAnalysisWidget->discardBogusOrders(discardBogusOrders);

        mOreReprocessingArbitrageWidget = new OreReprocessingArbitrageWidget{mDataProvider, *this, regionStationPresetRepository, tabs};
        mOreReprocessingArbitrageWidget->setPriceType(dst);
        connect(mOreReprocessingArbitrageWidget, &OreReprocessingArbitrageWidget::showInEve,
                this, &MarketAnalysisWidget::showInEve);

        mScrapmetalReprocessingArbitrageWidget = new ScrapmetalReprocessingArbitrageWidget{mDataProvider, *this, regionStationPresetRepository, tabs};
        mScrapmetalReprocessingArbitrageWidget->setPriceType(dst);
        connect(mScrapmetalReprocessingArbitrageWidget, &ScrapmetalReprocessingArbitrageWidget::showInEve,
                this, &MarketAnalysisWidget::showInEve);

        tabs->addTab(mRegionAnalysisWidget, tr("Region"));
        tabs->addTab(mInterRegionAnalysisWidget, tr("Inter-Region"));
        tabs->addTab(mImportingAnalysisWidget, tr("Importing"));
        tabs->addTab(mOreReprocessingArbitrageWidget, tr("Ore reprocessing arbitrage"));
        tabs->addTab(mScrapmetalReprocessingArbitrageWidget, tr("Scrapmetal reprocessing arbitrage"));
    }

    const MarketAnalysisWidget::HistoryMap *MarketAnalysisWidget::getHistory(uint regionId) const
    {
        if (!mHistory)
            return nullptr;

        const auto it = mHistory->find(regionId);
        return (it == std::end(*mHistory)) ? (nullptr) : (&it->second);
    }

    const MarketAnalysisWidget::HistoryRegionMap *MarketAnalysisWidget::getHistory() const
    {
        return mHistory.get();
    }

    const MarketAnalysisWidget::OrderResultType *MarketAnalysisWidget::getOrders() const
    {
        return (mOrders) ? (mOrders.get()) : (nullptr);
    }

    void MarketAnalysisWidget::setCharacter(Character::IdType id)
    {
        qDebug() << "Setting market analysis character to" << id;

        mCharacterId = id;

        const auto character = mCharacterRepo.find(mCharacterId);

        mRegionAnalysisWidget->setCharacter(character);
        mInterRegionAnalysisWidget->setCharacter(character);
        mImportingAnalysisWidget->setCharacter(character);
        mOreReprocessingArbitrageWidget->setCharacter(character);
        mScrapmetalReprocessingArbitrageWidget->setCharacter(character);
    }

    void MarketAnalysisWidget::prepareOrderImport()
    {
        RegionTypeSelectDialog dlg{mDataProvider, mTypeRepo, mGroupRepo, mRegionTypePresetRepo, this};
        connect(&dlg, &RegionTypeSelectDialog::selected, this, &MarketAnalysisWidget::importData);

        dlg.exec();
    }

    void MarketAnalysisWidget::importData(const TypeLocationPairs &pairs)
    {
        mOrders = std::make_shared<MarketAnalysisDataFetcher::OrderResultType::element_type>();
        mHistory = std::make_shared<MarketAnalysisDataFetcher::HistoryResultType::element_type>();

        mInterRegionAnalysisWidget->clearData();
        mImportingAnalysisWidget->clearData();
        mOreReprocessingArbitrageWidget->clearData();
        mScrapmetalReprocessingArbitrageWidget->clearData();

        if (!mDataFetcher.hasPendingOrderRequests() && !mDataFetcher.hasPendingHistoryRequests())
        {
            const auto mainTask = mTaskManager.startTask(tr("Importing data for analysis..."));

            mOrderSubtask = mTaskManager.startTask(mainTask, QStringLiteral("Making %1 order requests...").arg(pairs.size()));
            mHistorySubtask = mTaskManager.startTask(mainTask, tr("Making %1 history requests...").arg(pairs.size()));
        }

        TypeLocationPairs ignored;
        if (mIgnoreExistingOrdersBtn->isChecked())
        {
            auto ignoreTypes = [&](const auto &activeTypes) {
                for (const auto &pair : activeTypes)
                    ignored.insert(std::make_pair(pair.first, mDataProvider.getStationRegionId(pair.second)));
            };

            ignoreTypes(mOrderRepo.fetchActiveTypes());
            ignoreTypes(mCorpOrderRepo.fetchActiveTypes());
        }

        QMetaObject::invokeMethod(&mDataFetcher,
                                  "importData",
                                  Qt::QueuedConnection,
                                  Q_ARG(TypeLocationPairs, pairs),
                                  Q_ARG(TypeLocationPairs, ignored),
                                  Q_ARG(Character::IdType, mCharacterId));
    }

    void MarketAnalysisWidget::storeOrders()
    {
        emit updateExternalOrders(*mOrders);

        mTaskManager.endTask(mOrderSubtask);
        checkCompletion();
    }

    void MarketAnalysisWidget::showForCurrentRegion()
    {
        Q_ASSERT(mRegionAnalysisWidget != nullptr);
        mRegionAnalysisWidget->showForCurrentRegion();
    }

    void MarketAnalysisWidget::updateOrderTask(const QString &text)
    {
        mTaskManager.updateTask(mOrderSubtask, text);
    }

    void MarketAnalysisWidget::updateHistoryTask(const QString &text)
    {
        mTaskManager.updateTask(mHistorySubtask, text);
    }

    void MarketAnalysisWidget::endOrderTask(const MarketAnalysisDataFetcher::OrderResultType &orders, const QString &error)
    {
        Q_ASSERT(orders);
        mOrders = orders;

        if (error.isEmpty())
        {
            if (!mDontSaveBtn->isChecked())
            {
                mTaskManager.updateTask(mOrderSubtask, tr("Saving %1 imported orders...").arg(mOrders->size()));
                QMetaObject::invokeMethod(this, "storeOrders", Qt::QueuedConnection);
            }
            else
            {
                mTaskManager.endTask(mOrderSubtask);
                checkCompletion();
            }
        }
        else
        {
            mTaskManager.endTask(mOrderSubtask, error);
        }
    }

    void MarketAnalysisWidget::endHistoryTask(const MarketAnalysisDataFetcher::HistoryResultType &history, const QString &error)
    {
        Q_ASSERT(history);
        mHistory = history;

        if (error.isEmpty())
        {
            mTaskManager.endTask(mHistorySubtask);
            checkCompletion();
        }
        else
        {
            mTaskManager.endTask(mHistorySubtask, error);
        }
    }

    void MarketAnalysisWidget::checkCompletion()
    {
        if (!mDataFetcher.hasPendingOrderRequests() && !mDataFetcher.hasPendingHistoryRequests())
        {
            showForCurrentRegion();
            mInterRegionAnalysisWidget->completeImport();
            mImportingAnalysisWidget->completeImport();
        }
    }

    void MarketAnalysisWidget::recalculateAllData()
    {
        showForCurrentRegion();

        mInterRegionAnalysisWidget->recalculateAllData();
        mImportingAnalysisWidget->recalculateData();
        mOreReprocessingArbitrageWidget->recalculateData();
        mScrapmetalReprocessingArbitrageWidget->recalculateData();
    }
}
