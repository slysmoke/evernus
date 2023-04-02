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

#include <QWidgetAction>
#include <QApplication>
#include <QActionGroup>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QClipboard>
#include <QSettings>
#include <QCursor>
#include <QAction>
#include <QLabel>
#include <QMenu>

#include "MarketOrderVolumeItemDelegate.h"
#include "MarketOrdersInfoWidget.h"
#include "MarketOrderInfoWidget.h"
#include "StationSelectDialog.h"
#include "LookupActionGroup.h"
#include "MarketOrderModel.h"
#include "StyledTreeView.h"
#include "PriceSettings.h"

#include "MarketOrderView.h"

namespace Evernus
{
    MarketOrderView::MarketOrderView(const EveDataProvider &dataProvider,
                                     const QString &objectName,
                                     MarketOrdersInfoWidget *infoWidget,
                                     const ItemCostProvider &itemCostProvider,
                                     QWidget *parent)
        : QWidget{parent}
        , EveTypeProvider{}
        , mDataProvider{dataProvider}
        , mInfoWidget{infoWidget}
        , mProxy{mDataProvider, itemCostProvider}
    {
        auto mainLayout = new QVBoxLayout{this};

        mProxy.setSortRole(Qt::UserRole);
        mProxy.setFilterCaseSensitivity(Qt::CaseInsensitive);
        mProxy.setFilterKeyColumn(-1);
        connect(this, &MarketOrderView::statusFilterChanged, &mProxy, &MarketOrderFilterProxyModel::setStatusFilter);
        connect(this, &MarketOrderView::priceStatusFilterChanged, &mProxy, &MarketOrderFilterProxyModel::setPriceStatusFilter);
        connect(this, &MarketOrderView::textFilterChanged, &mProxy, &MarketOrderFilterProxyModel::setTextFilter);
        connect(&mProxy, &MarketOrderFilterProxyModel::scriptError, this, &MarketOrderView::scriptError);

        mView = new StyledTreeView{objectName, this};
        mainLayout->addWidget(mView, 1);
        mView->setModel(&mProxy);
        connect(mView, &StyledTreeView::clicked, this, &MarketOrderView::showPriceInfo);
        connect(getSelectionModel(), &QItemSelectionModel::selectionChanged,
                this, &MarketOrderView::selectOrder);

        mRemoveOrderAct = new QAction{QIcon{":/images/delete.png"}, tr("Delete order"), this};
        mRemoveOrderAct->setEnabled(false);
        mView->addAction(mRemoveOrderAct);
        connect(mRemoveOrderAct, &QAction::triggered, this, &MarketOrderView::removeOrders);

        mShowExternalOrdersAct = new QAction{tr("Show in market browser"), this};
        mShowExternalOrdersAct->setEnabled(false);
        mView->addAction(mShowExternalOrdersAct);
        connect(mShowExternalOrdersAct, &QAction::triggered, this, &MarketOrderView::showExternalOrdersForCurrent);

        mShowInEveAct = new QAction{tr("Show in EVE"), this};
        mShowInEveAct->setEnabled(false);
        mView->addAction(mShowInEveAct);
        connect(mShowInEveAct, &QAction::triggered, this, &MarketOrderView::showInEveForCurrent);

        mChangeNotesAct = new QAction{tr("Change notes"), this};
        mChangeNotesAct->setEnabled(false);
        mView->addAction(mChangeNotesAct);
        connect(mChangeNotesAct, &QAction::triggered, this, &MarketOrderView::changeNotesForCurrent);

        mChangeCustomStationAct = new QAction{tr("Set custom station"), this};
        mChangeCustomStationAct->setEnabled(false);
        mView->addAction(mChangeCustomStationAct);
        connect(mChangeCustomStationAct, &QAction::triggered, this, &MarketOrderView::changeCustomStationForCurrent);

        auto colorTagMenu = new QMenu{this};
        for (const auto &name : QColor::colorNames())
        {
            auto actionlabel = new QLabel{name, this};
            actionlabel->setStyleSheet(QStringLiteral("background-color: %1").arg(name));

            auto colorAction = new QWidgetAction{colorTagMenu};
            colorAction->setDefaultWidget(actionlabel);
            connect(colorAction, &QAction::triggered, this, [=] {
                changeColorTagForCurrent(QColor{name});
            });

            colorTagMenu->addAction(colorAction);
        }

        mChangeColorTagAct = new QAction{tr("Color tag"), this};
        mChangeColorTagAct->setEnabled(false);
        mChangeColorTagAct->setMenu(colorTagMenu);
        mView->addAction(mChangeColorTagAct);

        mRemoveColorTagAct = new QAction{tr("Remove color tag"), this};
        mRemoveColorTagAct->setEnabled(false);
        mView->addAction(mRemoveColorTagAct);
        connect(mRemoveColorTagAct, &QAction::triggered, this, [=] {
            changeColorTagForCurrent(QColor{});
        });

        if (mInfoWidget != nullptr)
            mainLayout->addWidget(mInfoWidget);

        mLookupGroup = new LookupActionGroup{*this, this};
        mLookupGroup->setEnabled(false);

        const auto action = new QAction{this};
        action->setSeparator(true);

        mView->addAction(action);
        mView->addActions(mLookupGroup->actions());
    }

    EveType::IdType MarketOrderView::getTypeId() const
    {
        return (mSource == nullptr) ? (EveType::invalidId) : (mSource->getOrderTypeId(mProxy.mapToSource(mView->currentIndex())));
    }

    QItemSelectionModel *MarketOrderView::getSelectionModel() const
    {
        return mView->selectionModel();
    }

    const QAbstractProxyModel &MarketOrderView::getProxyModel() const
    {
        return mProxy;
    }

    void MarketOrderView::setModel(MarketOrderModel *model)
    {
        if (mSource == model)
            return;

        mSource = model;

        auto curModel = mProxy.sourceModel();
        if (curModel != nullptr)
        {
            curModel->disconnect(this, SLOT(handleReset()));

            if (mInfoWidget != nullptr)
                curModel->disconnect(mInfoWidget, SLOT(updateData()));
        }

        mProxy.setSourceModel(mSource);
        mView->restoreHeaderState();

        if (mSource != nullptr)
        {
            mView->setItemDelegateForColumn(mSource->getVolumeColumn(), new MarketOrderVolumeItemDelegate{this});

            connect(mSource, &MarketOrderModel::modelReset, this, &MarketOrderView::handleReset);

            if (mInfoWidget != nullptr)
                connect(mSource, &MarketOrderModel::modelReset, mInfoWidget, &MarketOrdersInfoWidget::updateData);
        }

        if (mInfoWidget != nullptr)
            mInfoWidget->updateData();

        handleReset();
    }

    void MarketOrderView::expandAll()
    {
        mView->expandAll();
        mView->header()->resizeSections(QHeaderView::ResizeToContents);
    }

    void MarketOrderView::sortByColumn(int column, Qt::SortOrder order)
    {
        mView->sortByColumn(column, order);
    }

    void MarketOrderView::handleReset()
    {
        emit closeOrderInfo();
        mView->restoreHeaderState();
    }

    void MarketOrderView::changeNotesForCurrent()
    {
        const auto selection = getSelectionModel()->selectedRows();
        if (selection.isEmpty())
            return;

        QString initialText;
        if (selection.size() == 1)
        {
            const auto order = mSource->getOrder(mProxy.mapToSource(selection.first()));
            if (order == nullptr)
                return;

            initialText = order->getNotes();
        }

        const auto text = QInputDialog::getText(this, tr("Notes"), tr("Notes:"), QLineEdit::Normal, initialText);
        if (!text.isNull())
        {
            for (const auto &index : selection)
            {
                const auto order = mSource->getOrder(mProxy.mapToSource(index));
                if (order != nullptr)
                    emit notesChanged(order->getId(), text);
            }
        }
    }

    void MarketOrderView::changeCustomStationForCurrent()
    {
        const auto selection = getSelectionModel()->selectedRows();
        if (selection.isEmpty())
            return;

        StationSelectDialog dlg{mDataProvider};
        if (dlg.exec() == QDialog::Accepted)
        {
            const auto id = dlg.getStationId();
            for (const auto &index : selection)
            {
                const auto order = mSource->getOrder(mProxy.mapToSource(index));
                if (order != nullptr)
                    emit stationChanged(order->getId(), id);
            }
        }
    }

    void MarketOrderView::changeColorTagForCurrent(const QColor &color)
    {
        const auto selection = getSelectionModel()->selectedRows();
        if (selection.isEmpty())
            return;

        for (const auto &index : selection)
        {
            const auto order = mSource->getOrder(mProxy.mapToSource(index));
            if (order != nullptr)
                emit colorTagChanged(order->getId(), color);
        }
    }

    void MarketOrderView::executeFPC()
    {
        executeFPC(1);
    }

    void MarketOrderView::executeBackwardFPC()
    {
        executeFPC(-1);
    }

    void MarketOrderView::showPriceInfo(const QModelIndex &index)
    {
        emit closeOrderInfo();
        emit itemSelected();

        const auto source = mProxy.mapToSource(index);

        if (!mSource->shouldShowPriceInfo(source))
            return;

        auto infoWidget = new MarketOrderInfoWidget{mSource->getOrderInfo(source), this};
        infoWidget->move(QCursor::pos());
        infoWidget->show();
        infoWidget->activateWindow();
        connect(this, &MarketOrderView::closeOrderInfo, infoWidget, &MarketOrderInfoWidget::deleteLater);
    }

    void MarketOrderView::selectOrder(const QItemSelection &selected)
    {
        if (mSource == nullptr)
            return;

        const auto enable = !selected.isEmpty() && mSource->getOrder(mProxy.mapToSource(mView->currentIndex())) != nullptr;
        mRemoveOrderAct->setEnabled(enable);
        mShowExternalOrdersAct->setEnabled(enable);
        mShowInEveAct->setEnabled(enable);
        mChangeNotesAct->setEnabled(enable);
        mChangeCustomStationAct->setEnabled(enable);
        mChangeColorTagAct->setEnabled(enable);
        mRemoveColorTagAct->setEnabled(enable);
        mLookupGroup->setEnabled(enable);

        emit itemSelected();
    }

    void MarketOrderView::removeOrders()
    {
        if (mSource == nullptr)
            return;

        auto indexes = getSelectionModel()->selectedRows();
        std::transform(std::begin(indexes), std::end(indexes), std::begin(indexes), [=](const auto &index) {
            return mProxy.mapToSource(index);
        });

        mSource->removeIndexes(indexes);
    }

    void MarketOrderView::showExternalOrdersForCurrent()
    {
        if (mSource == nullptr)
            return;

        const auto typeId = mSource->getOrderTypeId(mProxy.mapToSource(mView->currentIndex()));
        if (typeId != EveType::invalidId)
            emit showExternalOrders(typeId);
    }

    void MarketOrderView::showInEveForCurrent()
    {
        showInEveFor(mProxy.mapToSource(mView->currentIndex()));
    }

    void MarketOrderView::showInEveFor(const QModelIndex &index) const
    {
        if (mSource == nullptr)
            return;

        const auto typeId = mSource->getOrderTypeId(index);
        if (typeId != EveType::invalidId)
            emit showInEve(typeId, mSource->getOrderOwnerId(index));
    }

    void MarketOrderView::executeFPC(int delta)
    {
        if (mSource != nullptr)
        {
            auto model = getSelectionModel();

            const auto selection = model->selectedRows();
            if (selection.isEmpty())
                return;

            const auto selected = selection.first();
            const auto source = mProxy.mapToSource(selected);
            const auto next = mProxy.index(selected.row() + delta, 0, selected.parent());
            if (next.isValid())
            {
                model->select(next, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
                mView->scrollTo(next);
            }

            const auto info = mSource->getOrderInfo(source);
            if (info.mTargetPrice >= 0.01)
                QApplication::clipboard()->setText(QString::number(info.mTargetPrice, 'f', 2));

            QSettings settings;
            if (settings.value(PriceSettings::showInEveOnFpcKey, PriceSettings::showInEveOnFpcDefault).toBool())
                showInEveFor(source);
        }
    }
}
