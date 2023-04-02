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
#include <QHeaderView>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QGroupBox>

#include "ExternalOrderSellModel.h"
#include "ExternalOrderBuyModel.h"
#include "WalletTransactionView.h"
#include "ExternalOrderView.h"
#include "ItemCostProvider.h"
#include "MarketOrderModel.h"
#include "MarketOrderView.h"

#include "MarketOrderViewWithTransactions.h"

namespace Evernus
{
    MarketOrderViewWithTransactions::MarketOrderViewWithTransactions(const WalletTransactionRepository &transactionsRepo,
                                                                     const CharacterRepository &characterRepo,
                                                                     const ExternalOrderRepository &externalOrderRepo,
                                                                     const EveDataProvider &dataProvider,
                                                                     ItemCostProvider &costProvider,
                                                                     const MarketOrderProvider &orderProvider,
                                                                     const MarketOrderProvider &corpOrderProvider,
                                                                     bool corp,
                                                                     const QString &objectName,
                                                                     MarketOrdersInfoWidget *infoWidget,
                                                                     bool showExternalOrders,
                                                                     QWidget *parent)
        : QWidget(parent)
        , mCharacterRepo(characterRepo)
        , mExternalOrderRepo(externalOrderRepo)
        , mDataProvider(dataProvider)
        , mCostProvider(costProvider)
        , mOrderProvider(orderProvider)
        , mCorpOrderProvider(corpOrderProvider)
        , mTransactionModel(transactionsRepo, mCharacterRepo, mDataProvider, mCostProvider, corp)
    {
        auto mainLayout = new QVBoxLayout{this};

        mOrderView = new MarketOrderView{mDataProvider, objectName + "-orders", infoWidget, costProvider, this};
        mainLayout->addWidget(mOrderView, 1);
        connect(this, &MarketOrderViewWithTransactions::statusFilterChanged, mOrderView, &MarketOrderView::statusFilterChanged);
        connect(this, &MarketOrderViewWithTransactions::priceStatusFilterChanged, mOrderView, &MarketOrderView::priceStatusFilterChanged);
        connect(this, &MarketOrderViewWithTransactions::textFilterChanged, mOrderView, &MarketOrderView::textFilterChanged);
        connect(mOrderView, &MarketOrderView::scriptError, this, &MarketOrderViewWithTransactions::scriptError);
        connect(mOrderView, &MarketOrderView::showExternalOrders, this, &MarketOrderViewWithTransactions::showExternalOrders);
        connect(mOrderView, &MarketOrderView::showInEve, this, &MarketOrderViewWithTransactions::showInEve);
        connect(mOrderView, &MarketOrderView::itemSelected, this, &MarketOrderViewWithTransactions::itemSelected);
        connect(mOrderView, &MarketOrderView::notesChanged, this, &MarketOrderViewWithTransactions::notesChanged);
        connect(mOrderView, &MarketOrderView::stationChanged, this, &MarketOrderViewWithTransactions::stationChanged);
        connect(mOrderView, &MarketOrderView::colorTagChanged, this, &MarketOrderViewWithTransactions::colorTagChanged);
        connect(mOrderView->getSelectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MarketOrderViewWithTransactions::selectOrder);

        auto transactionGroup = new QGroupBox{this};
        mainLayout->addWidget(transactionGroup);

        auto groupLayout = new QVBoxLayout{transactionGroup};

        mTransactionProxyModel.setSortRole(Qt::UserRole);
        mTransactionProxyModel.setSourceModel(&mTransactionModel);

        auto tabs = new QTabWidget{this};
        groupLayout->addWidget(tabs);
        tabs->setTabPosition(QTabWidget::West);

        if (showExternalOrders)
        {
            mExternalOrderView = new ExternalOrderView{mCostProvider, mDataProvider, objectName + "-externalOrders", this};
            tabs->addTab(mExternalOrderView, tr("Market orders"));
        }

        mTransactionsView = new WalletTransactionView{objectName + "-transactions",
                                                      mCostProvider,
                                                      mCharacterRepo,
                                                      this};
        tabs->addTab(mTransactionsView, tr("Transactions"));
        connect(mTransactionsView, &WalletTransactionView::showInEve, this, &MarketOrderViewWithTransactions::showInEve);
        mTransactionsView->setModels(&mTransactionProxyModel, &mTransactionModel);
        mTransactionsView->sortByColumn(1, Qt::DescendingOrder);
        mTransactionsView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    }

    void MarketOrderViewWithTransactions::setModel(MarketOrderModel *model)
    {
        mOrderModel = model;
        mOrderView->setModel(mOrderModel);

        if (mOrderModel == nullptr)
        {
            mExternalOrderModel.reset();
        }
        else
        {
            switch (mOrderModel->getType()) {
            case MarketOrderModel::Type::Buy:
                mExternalOrderModel = std::make_unique<ExternalOrderBuyModel>(mDataProvider,
                                                                              mExternalOrderRepo,
                                                                              mCharacterRepo,
                                                                              mOrderProvider,
                                                                              mCorpOrderProvider,
                                                                              mCostProvider);
                break;
            case MarketOrderModel::Type::Sell:
                mExternalOrderModel = std::make_unique<ExternalOrderSellModel>(mDataProvider,
                                                                               mExternalOrderRepo,
                                                                               mCharacterRepo,
                                                                               mOrderProvider,
                                                                               mCorpOrderProvider,
                                                                               mCostProvider);
                break;
            default:
                mExternalOrderModel.reset();
            }

            if (mExternalOrderModel)
            {
                mExternalOrderModel->setPriceColorMode(ExternalOrderModel::PriceColorMode::Deviation);
                connect(mOrderModel, &MarketOrderModel::modelReset, mExternalOrderModel.get(), &ExternalOrderModel::reset);
            }
        }

        if (mExternalOrderView != nullptr)
            mExternalOrderView->setModel(mExternalOrderModel.get());
    }

    void MarketOrderViewWithTransactions::setCharacter(Character::IdType id)
    {
        mCharacterId = id;
        mTransactionModel.clear();

        mTransactionsView->setCharacter(id);

        if (mExternalOrderModel)
            mExternalOrderModel->setCharacter(mCharacterId);
    }

    void MarketOrderViewWithTransactions::expandAll()
    {
        mOrderView->expandAll();
    }

    void MarketOrderViewWithTransactions::sortByColumn(int column, Qt::SortOrder order)
    {
        mOrderView->sortByColumn(column, order);
    }

    void MarketOrderViewWithTransactions::executeFPC()
    {
        mOrderView->executeFPC();
    }

    void MarketOrderViewWithTransactions::executeBackwardFPC()
    {
        mOrderView->executeBackwardFPC();
    }

    void MarketOrderViewWithTransactions::updateCharacters()
    {
        mTransactionsView->updateCharacters();
    }

    void MarketOrderViewWithTransactions::selectOrder(const QItemSelection &selected)
    {
        if (mOrderModel != nullptr && !selected.isEmpty())
        {
            const auto index = mOrderView->getProxyModel().mapToSource(selected.indexes().first());
            const auto order = mOrderModel->getOrder(index);

            if (order == nullptr || order->getTypeId() == EveType::invalidId)
            {
                mTransactionModel.clear();
            }
            else
            {
                auto range = mOrderModel->getOrderRange(index);

                if (!range.mTo.isValid())
                    range.mTo = QDateTime::currentDateTimeUtc();

                mTransactionModel.setFilter(
                    mCharacterId, range.mFrom.date(), range.mTo.date(), mOrderModel->getOrderTypeFilter(index), false, order->getTypeId());
            }

            if (mExternalOrderModel)
            {
                mExternalOrderModel->blockSignals(true);

                if (order == nullptr)
                {
                    mExternalOrderModel->setTypeId(EveType::invalidId);
                    mExternalOrderModel->setStationId(0);
                }
                else
                {
                    mExternalOrderModel->setTypeId(order->getTypeId());
                    mExternalOrderModel->setStationId(order->getStationId());
                    mExternalOrderModel->changeDeviationSource(ExternalOrderModel::DeviationSourceType::Fixed, order->getPrice());
                }

                mExternalOrderModel->blockSignals(false);

                mExternalOrderModel->reset();
            }
        }
    }
}
