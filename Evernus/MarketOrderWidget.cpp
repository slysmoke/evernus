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
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QSettings>
#include <QLabel>
#include <QtDebug>
#include <QMenu>

#include "MarketOrderViewWithTransactions.h"
#include "GenericMarketOrdersInfoWidget.h"
#include "SellMarketOrdersInfoWidget.h"
#include "MarketOrderFilterWidget.h"
#include "MarketOrderProvider.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "MarketOrderView.h"
#include "DateRangeWidget.h"
#include "ButtonWithTimer.h"
#include "ImportSettings.h"

#include "MarketOrderWidget.h"

namespace Evernus
{
    const QString MarketOrderWidget::settingsLastTabKey = "ui/orders/lastTab";
    const QString MarketOrderWidget::settingsShowForAllKey = "ui/orders/showForAllCharacters";

    MarketOrderWidget::MarketOrderWidget(MarketOrderProvider &orderProvider,
                                         MarketOrderProvider &corpOrderProvider,
                                         const CacheTimerProvider &cacheTimerProvider,
                                         const EveDataProvider &dataProvider,
                                         ItemCostProvider &itemCostProvider,
                                         const WalletTransactionRepository &transactionsRepo,
                                         const CharacterRepository &characterRepository,
                                         const FilterTextRepository &filterRepo,
                                         const ExternalOrderRepository &externalOrderRepo,
                                         bool corp,
                                         QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpMarketOrders) : (TimerType::MarketOrders)),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpMarketOrders) : (TimerType::MarketOrders)),
                               ImportSettings::maxMarketOrdersAgeKey,
                               parent)
        , mOrderProvider(orderProvider)
        , mCharacterRepository(characterRepository)
        , mSellModel(mOrderProvider, dataProvider, itemCostProvider, cacheTimerProvider, mCharacterRepository, corp)
        , mBuyModel(mOrderProvider, dataProvider, cacheTimerProvider, mCharacterRepository, corp)
        , mArchiveModel(mOrderProvider, dataProvider, itemCostProvider, mCharacterRepository, corp)
        , mCorp(corp)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        mLogImportBtn = new QPushButton{QIcon{":/images/page_refresh.png"}, tr("File import"), this};
        toolBarLayout->addWidget(mLogImportBtn);
        mLogImportBtn->setEnabled(false);
        mLogImportBtn->setFlat(true);
        connect(mLogImportBtn, &QPushButton::clicked, this, [this]() {
            emit importFromLogs(getCharacterId());
        });

        auto importMenu = new QMenu{this};

        importMenu->addAction(QIcon{":/images/world.png"}, tr("Import prices from Web"), this, &MarketOrderWidget::prepareItemImportFromWeb);
        importMenu->addAction(QIcon{":/images/page_refresh.png"}, tr("Import prices from logs"), this, &MarketOrderWidget::prepareItemImportFromFile);

        auto allImportBtn = new QPushButton{QIcon{":/images/arrow_refresh_small.png"}, tr("Import prices  "), this};
        toolBarLayout->addWidget(allImportBtn);
        allImportBtn->setFlat(true);
        allImportBtn->setMenu(importMenu);

        auto openMarginTool = new QPushButton{QIcon{":/images/report.png"}, tr("Open margin tool"), this};
        toolBarLayout->addWidget(openMarginTool);
        openMarginTool->setFlat(true);
        connect(openMarginTool, &QPushButton::clicked, this, &MarketOrderWidget::openMarginTool);

        auto stateFilter = new MarketOrderFilterWidget{filterRepo, this};
        toolBarLayout->addWidget(stateFilter);

        QSettings settings;

        mShowForAllCharacters = new QCheckBox{tr("Show for all characters"), this};
        toolBarLayout->addWidget(mShowForAllCharacters);
        connect(mShowForAllCharacters, &QCheckBox::stateChanged, this, [=](int state) {
            const auto checked = state == Qt::Checked;

            mSellModel.setAllCaracters(checked);
            mBuyModel.setAllCaracters(checked);
            mArchiveModel.setAllCaracters(checked);

            QSettings settings;
            settings.setValue(settingsShowForAllKey, checked);
        });
        mShowForAllCharacters->setChecked(settings.value(settingsShowForAllKey, false).toBool());

        toolBarLayout->addWidget(new QLabel{tr("Group by:"), this});

        mGroupingCombo = new QComboBox{this};
        toolBarLayout->addWidget(mGroupingCombo);
        mGroupingCombo->addItem(tr("- none -"), static_cast<int>(MarketOrderModel::Grouping::None));
        mGroupingCombo->addItem(tr("Type"), static_cast<int>(MarketOrderModel::Grouping::Type));
        mGroupingCombo->addItem(tr("Group"), static_cast<int>(MarketOrderModel::Grouping::Group));
        mGroupingCombo->addItem(tr("Station"), static_cast<int>(MarketOrderModel::Grouping::Station));
        connect(mGroupingCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &MarketOrderWidget::changeGrouping);

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        auto mainTabs = new QTabWidget{this};
        mainLayout->addWidget(mainTabs);

        mSellView = new MarketOrderViewWithTransactions{transactionsRepo,
                                                        characterRepository,
                                                        externalOrderRepo,
                                                        dataProvider,
                                                        itemCostProvider,
                                                        mOrderProvider,
                                                        corpOrderProvider,
                                                        corp,
                                                        (mCorp) ? ("corpMarketOrderSellView") : ("marketOrderSellView"),
                                                        new SellMarketOrdersInfoWidget{mSellModel, this},
                                                        true,
                                                        this};
        mainTabs->addTab(mSellView, QIcon{":/images/arrow_out.png"}, tr("Sell"));
        mSellView->setModel(&mSellModel);
        mSellView->sortByColumn(0, Qt::AscendingOrder);
        connect(this, &MarketOrderWidget::characterChanged, mSellView, &MarketOrderViewWithTransactions::setCharacter);
        connect(mSellView, &MarketOrderViewWithTransactions::scriptError, this, &MarketOrderWidget::showScriptError);
        connect(mSellView, &MarketOrderViewWithTransactions::showExternalOrders, this, &MarketOrderWidget::showExternalOrders);
        connect(mSellView, &MarketOrderViewWithTransactions::showInEve, this, &MarketOrderWidget::showInEve);
        connect(mSellView, &MarketOrderViewWithTransactions::itemSelected, this, &MarketOrderWidget::changeFPCExecutor);
        connect(stateFilter, &MarketOrderFilterWidget::statusFilterChanged, mSellView, &MarketOrderViewWithTransactions::statusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::priceStatusFilterChanged, mSellView, &MarketOrderViewWithTransactions::priceStatusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::textFilterChanged, mSellView, &MarketOrderViewWithTransactions::textFilterChanged);

        mBuyView = new MarketOrderViewWithTransactions{transactionsRepo,
                                                       characterRepository,
                                                       externalOrderRepo,
                                                       dataProvider,
                                                       itemCostProvider,
                                                       mOrderProvider,
                                                       corpOrderProvider,
                                                       corp,
                                                       (mCorp) ? ("corpMarketOrderBuyView") : ("marketOrderBuyView"),
                                                       new GenericMarketOrdersInfoWidget{mBuyModel, this},
                                                       true,
                                                       this};
        mainTabs->addTab(mBuyView, QIcon{":/images/arrow_in.png"}, tr("Buy"));
        mBuyView->setModel(&mBuyModel);
        mBuyView->sortByColumn(0, Qt::AscendingOrder);
        connect(this, &MarketOrderWidget::characterChanged, mBuyView, &MarketOrderViewWithTransactions::setCharacter);
        connect(mBuyView, &MarketOrderViewWithTransactions::showExternalOrders, this, &MarketOrderWidget::showExternalOrders);
        connect(mBuyView, &MarketOrderViewWithTransactions::showInEve, this, &MarketOrderWidget::showInEve);
        connect(mBuyView, &MarketOrderViewWithTransactions::itemSelected, this, &MarketOrderWidget::changeFPCExecutor);
        connect(stateFilter, &MarketOrderFilterWidget::statusFilterChanged, mBuyView, &MarketOrderViewWithTransactions::statusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::priceStatusFilterChanged, mBuyView, &MarketOrderViewWithTransactions::priceStatusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::textFilterChanged, mBuyView, &MarketOrderViewWithTransactions::textFilterChanged);

        auto combinedWidget = new QWidget{this};
        mainTabs->addTab(combinedWidget, QIcon{":/images/arrow_inout.png"}, tr("Sell && Buy"));

        auto combinedLayout = new QVBoxLayout{combinedWidget};

        auto sellGroup = new QGroupBox{tr("Sell orders"), this};
        combinedLayout->addWidget(sellGroup);

        auto sellGroupLayout = new QVBoxLayout{sellGroup};

        mCombinedSellView = new MarketOrderView{dataProvider,
                                                (mCorp) ? ("corpCombinedMarketOrderSellView") : ("combinedMarketOrderSellView"),
                                                new SellMarketOrdersInfoWidget{mSellModel, this},
                                                itemCostProvider,
                                                this};
        sellGroupLayout->addWidget(mCombinedSellView);
        mCombinedSellView->setModel(&mSellModel);
        mCombinedSellView->sortByColumn(0, Qt::AscendingOrder);
        connect(mCombinedSellView, &MarketOrderView::showExternalOrders, this, &MarketOrderWidget::showExternalOrders);
        connect(mCombinedSellView, &MarketOrderView::showInEve, this, &MarketOrderWidget::showInEve);
        connect(mCombinedSellView, &MarketOrderView::itemSelected, this, &MarketOrderWidget::changeFPCExecutor);
        connect(stateFilter, &MarketOrderFilterWidget::statusFilterChanged, mCombinedSellView, &MarketOrderView::statusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::priceStatusFilterChanged, mCombinedSellView, &MarketOrderView::priceStatusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::textFilterChanged, mCombinedSellView, &MarketOrderView::textFilterChanged);

        auto buyGroup = new QGroupBox{tr("Buy orders"), this};
        combinedLayout->addWidget(buyGroup);

        auto buyGroupLayout = new QVBoxLayout{buyGroup};

        mCombinedBuyView = new MarketOrderView{dataProvider,
                                               (mCorp) ? ("corpCombinedMarketOrderBuyView") : ("combinedMarketOrderBuyView"),
                                               new GenericMarketOrdersInfoWidget{mBuyModel, this},
                                               itemCostProvider,
                                               this};
        buyGroupLayout->addWidget(mCombinedBuyView);
        mCombinedBuyView->setModel(&mBuyModel);
        mCombinedBuyView->sortByColumn(0, Qt::AscendingOrder);
        connect(mCombinedBuyView, &MarketOrderView::showExternalOrders, this, &MarketOrderWidget::showExternalOrders);
        connect(mCombinedBuyView, &MarketOrderView::showInEve, this, &MarketOrderWidget::showInEve);
        connect(mCombinedBuyView, &MarketOrderView::itemSelected, this, &MarketOrderWidget::changeFPCExecutor);
        connect(stateFilter, &MarketOrderFilterWidget::statusFilterChanged, mCombinedBuyView, &MarketOrderView::statusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::priceStatusFilterChanged, mCombinedBuyView, &MarketOrderView::priceStatusFilterChanged);
        connect(stateFilter, &MarketOrderFilterWidget::textFilterChanged, mCombinedBuyView, &MarketOrderView::textFilterChanged);

        auto archiveTab = new QWidget{this};
        mainTabs->addTab(archiveTab, QIcon{":/images/hourglass.png"}, tr("History"));

        auto archiveLayout = new QVBoxLayout{archiveTab};

        auto rangeLayout = new QHBoxLayout{};
        archiveLayout->addLayout(rangeLayout);

        mArchiveRangeEdit = new DateRangeWidget{this};
        rangeLayout->addWidget(mArchiveRangeEdit);
        connect(mArchiveRangeEdit, &DateRangeWidget::rangeChanged, this, &MarketOrderWidget::setArchiveRange);

        rangeLayout->addStretch();

        mArchiveView = new MarketOrderViewWithTransactions{transactionsRepo,
                                                           characterRepository,
                                                           externalOrderRepo,
                                                           dataProvider,
                                                           itemCostProvider,
                                                           mOrderProvider,
                                                           corpOrderProvider,
                                                           corp,
                                                           (mCorp) ? ("corpMarketOrderArchiveView") : ("marketOrderArchiveView"),
                                                           nullptr,
                                                           false,
                                                           this};
        archiveLayout->addWidget(mArchiveView);
        mArchiveView->statusFilterChanged(MarketOrderFilterProxyModel::EveryStatus);
        mArchiveView->priceStatusFilterChanged(MarketOrderFilterProxyModel::EveryPriceStatus);
        mArchiveView->setModel(&mArchiveModel);
        mArchiveView->sortByColumn(0, Qt::DescendingOrder);
        connect(mArchiveView, &MarketOrderViewWithTransactions::showExternalOrders, this, &MarketOrderWidget::showExternalOrders);
        connect(mArchiveView, &MarketOrderViewWithTransactions::showInEve, this, &MarketOrderWidget::showInEve);
        connect(this, &MarketOrderWidget::characterChanged, mArchiveView, &MarketOrderViewWithTransactions::setCharacter);
        connect(stateFilter, &MarketOrderFilterWidget::textFilterChanged, mArchiveView, &MarketOrderViewWithTransactions::textFilterChanged);

        mainTabs->setCurrentIndex(settings.value(settingsLastTabKey).toInt());
        connect(mainTabs, &QTabWidget::currentChanged, this, &MarketOrderWidget::saveChosenTab);

        connect(this, &MarketOrderWidget::characterChanged, &mSellModel, &MarketOrderSellModel::setCharacter);
        connect(this, &MarketOrderWidget::characterChanged, &mBuyModel, &MarketOrderBuyModel::setCharacter);

        connect(mSellView, &MarketOrderViewWithTransactions::notesChanged, this, &MarketOrderWidget::setOrderNotes);
        connect(mBuyView, &MarketOrderViewWithTransactions::notesChanged, this, &MarketOrderWidget::setOrderNotes);
        connect(mArchiveView, &MarketOrderViewWithTransactions::notesChanged, this, &MarketOrderWidget::setOrderNotes);
        connect(mCombinedSellView, &MarketOrderView::notesChanged, this, &MarketOrderWidget::setOrderNotes);
        connect(mCombinedBuyView, &MarketOrderView::notesChanged, this, &MarketOrderWidget::setOrderNotes);
        connect(mSellView, &MarketOrderViewWithTransactions::stationChanged, this, &MarketOrderWidget::setOrderStation);
        connect(mBuyView, &MarketOrderViewWithTransactions::stationChanged, this, &MarketOrderWidget::setOrderStation);
        connect(mArchiveView, &MarketOrderViewWithTransactions::stationChanged, this, &MarketOrderWidget::setOrderStation);
        connect(mCombinedSellView, &MarketOrderView::stationChanged, this, &MarketOrderWidget::setOrderStation);
        connect(mCombinedBuyView, &MarketOrderView::stationChanged, this, &MarketOrderWidget::setOrderStation);
        connect(mSellView, &MarketOrderViewWithTransactions::colorTagChanged, this, &MarketOrderWidget::setColorTag);
        connect(mBuyView, &MarketOrderViewWithTransactions::colorTagChanged, this, &MarketOrderWidget::setColorTag);
        connect(mArchiveView, &MarketOrderViewWithTransactions::colorTagChanged, this, &MarketOrderWidget::setColorTag);
        connect(mCombinedSellView, &MarketOrderView::colorTagChanged, this, &MarketOrderWidget::setColorTag);
        connect(mCombinedBuyView, &MarketOrderView::colorTagChanged, this, &MarketOrderWidget::setColorTag);
    }

    void MarketOrderWidget::updateData()
    {
        refreshImportTimer();
        mSellModel.reset();
        mBuyModel.reset();
        mArchiveModel.reset();

        expandAll();
    }

    void MarketOrderWidget::updateCharacters()
    {
        mSellView->updateCharacters();
        mBuyView->updateCharacters();
        mArchiveView->updateCharacters();
    }

    void MarketOrderWidget::changeGrouping()
    {
        mSellModel.setGrouping(static_cast<MarketOrderModel::Grouping>(mGroupingCombo->currentData().toInt()));
        mBuyModel.setGrouping(static_cast<MarketOrderModel::Grouping>(mGroupingCombo->currentData().toInt()));
        mArchiveModel.setGrouping(static_cast<MarketOrderModel::Grouping>(mGroupingCombo->currentData().toInt()));

        expandAll();
    }

    void MarketOrderWidget::saveChosenTab(int index)
    {
        QSettings settings;
        settings.setValue(settingsLastTabKey, index);
    }

    void MarketOrderWidget::prepareItemImportFromWeb()
    {
        emit importPricesFromWeb(getCharacterId(), getImportTarget());
    }

    void MarketOrderWidget::prepareItemImportFromFile()
    {
        emit importPricesFromFile(getCharacterId(), getImportTarget());
    }

    void MarketOrderWidget::setArchiveRange(const QDate &from, const QDate &to)
    {
        mArchiveModel.setCharacterAndRange(getCharacterId(), QDateTime{from}.toUTC(), QDateTime{to}.addDays(1).toUTC());
    }

    void MarketOrderWidget::showScriptError(const QString &message)
    {
        QMessageBox::warning(this, tr("Script error"), message);
    }

    void MarketOrderWidget::changeFPCExecutor()
    {
        emit fpcExecutorChanged(sender());
    }

    void MarketOrderWidget::setOrderNotes(MarketOrder::IdType id, const QString &notes)
    {
        mOrderProvider.setOrderNotes(id, notes);
    }

    void MarketOrderWidget::setOrderStation(MarketOrder::IdType orderId, uint stationId)
    {
        mOrderProvider.setOrderStation(orderId, stationId);
    }

    void MarketOrderWidget::setColorTag(MarketOrder::IdType orderId, const QColor &color)
    {
        mOrderProvider.setOrderColorTag(orderId, color);
    }

    void MarketOrderWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching market orders to" << id;

        const auto tillDate = QDate::currentDate();
        const auto fromDate = tillDate.addDays(-7);

        mArchiveRangeEdit->blockSignals(true);
        mArchiveRangeEdit->setRange(fromDate, tillDate);
        mArchiveRangeEdit->blockSignals(false);

        mLogImportBtn->setEnabled(id != Character::invalidId);
        mArchiveModel.setCharacterAndRange(id, QDateTime{fromDate}.toUTC(), QDateTime{tillDate}.addDays(1).toUTC());

        emit characterChanged(id);
    }

    TypeLocationPairs MarketOrderWidget::getImportTarget() const
    {
        TypeLocationPairs target;

        const auto importer = [&target](const auto &orders) {
            for (const auto &order : orders)
            {
                Q_ASSERT(order);
                target.emplace(std::make_pair(order->getTypeId(), order->getEffectiveStationId()));
            }
        };

        if (mCorp)
        {
            try
            {
                const auto id = mCharacterRepository.getCorporationId(getCharacterId());

                importer(mOrderProvider.getBuyOrdersForCorporation(id));
                importer(mOrderProvider.getSellOrdersForCorporation(id));
            }
            catch (const CharacterRepository::NotFoundException &)
            {
            }
        }
        else
        {
            importer(mOrderProvider.getBuyOrders(getCharacterId()));
            importer(mOrderProvider.getSellOrders(getCharacterId()));
        }

        return target;
    }

    void MarketOrderWidget::expandAll()
    {
        mSellView->expandAll();
        mCombinedSellView->expandAll();
        mBuyView->expandAll();
        mCombinedBuyView->expandAll();
        mArchiveView->expandAll();
    }
}
