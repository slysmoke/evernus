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
#include <QWidgetAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTabWidget>
#include <QtDebug>
#include <QMenu>

#include "ContractStatusesWidget.h"
#include "CacheTimerProvider.h"
#include "WarningBarWidget.h"
#include "TextFilterWidget.h"
#include "ButtonWithTimer.h"
#include "ImportSettings.h"
#include "ContractView.h"

#include "ContractWidget.h"

namespace Evernus
{
    ContractWidget::ContractWidget(const CacheTimerProvider &cacheTimerProvider,
                                   const EveDataProvider &dataProvider,
                                   const ContractProvider &contractProvider,
                                   const FilterTextRepository &filterRepo,
                                   const CharacterRepository &characterRepo,
                                   bool corp,
                                   QWidget *parent)
        : CharacterBoundWidget(std::bind(&CacheTimerProvider::getLocalCacheTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpContracts) : (TimerType::Contracts)),
                               std::bind(&CacheTimerProvider::getLocalUpdateTimer, &cacheTimerProvider, std::placeholders::_1, (corp) ? (TimerType::CorpContracts) : (TimerType::Contracts)),
                               ImportSettings::maxContractsAgeKey,
                               parent)
        , mIssuedModel(dataProvider, contractProvider, characterRepo, corp)
        , mAssignedModel(dataProvider, contractProvider, characterRepo, corp)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto toolBarLayout = new QHBoxLayout{};
        mainLayout->addLayout(toolBarLayout);

        auto &importBtn = getAPIImportButton();
        toolBarLayout->addWidget(&importBtn);

        auto statusesWidget = new ContractStatusesWidget{this};
        const auto currentFilter = statusesWidget->getStatusFilter();
        connect(statusesWidget, &ContractStatusesWidget::filterChanged, this, &ContractWidget::setStatusFilter);

        auto filterStatusAction = new QWidgetAction{this};
        filterStatusAction->setDefaultWidget(statusesWidget);

        auto filterStatusMenu = new QMenu{this};
        filterStatusMenu->addAction(filterStatusAction);

        mStatusFilterBtn = new QPushButton{QIcon{":/images/flag_blue.png"}, getStatusFilterButtonText(currentFilter), this};
        toolBarLayout->addWidget(mStatusFilterBtn);
        mStatusFilterBtn->setFlat(true);
        mStatusFilterBtn->setMenu(filterStatusMenu);

        auto filterEdit = new TextFilterWidget{filterRepo, this};
        toolBarLayout->addWidget(filterEdit, 1);

        auto &warningBar = getWarningBarWidget();
        mainLayout->addWidget(&warningBar);

        auto tabs = new QTabWidget{this};
        mainLayout->addWidget(tabs, 1);

        auto issuedView = new ContractView{(corp) ? ("corpIssuedContractsView") : ("issuedContractsView"), this};
        tabs->addTab(issuedView, tr("Issued"));
        issuedView->setModel(&mIssuedModel);
        issuedView->setStatusFilter(currentFilter);
        connect(filterEdit, &TextFilterWidget::filterEntered, issuedView, &ContractView::setFilterWildcard);
        connect(statusesWidget, &ContractStatusesWidget::filterChanged, issuedView, &ContractView::setStatusFilter);

        auto assignedView = new ContractView{(corp) ? ("corpAssignedContractsView") : ("assignedContractsView"), this};
        tabs->addTab(assignedView, tr("Assigned"));
        assignedView->setModel(&mAssignedModel);
        assignedView->setStatusFilter(currentFilter);
        connect(filterEdit, &TextFilterWidget::filterEntered, assignedView, &ContractView::setFilterWildcard);
        connect(statusesWidget, &ContractStatusesWidget::filterChanged, assignedView, &ContractView::setStatusFilter);
    }

    void ContractWidget::updateData()
    {
        refreshImportTimer();
        mIssuedModel.reset();
        mAssignedModel.reset();
    }

    void ContractWidget::setStatusFilter(const ContractFilterProxyModel::StatusFilters &filter)
    {
        mStatusFilterBtn->setText(getStatusFilterButtonText(filter));
    }

    void ContractWidget::handleNewCharacter(Character::IdType id)
    {
        qDebug() << "Switching contracts to" << id;

        mIssuedModel.setCharacter(id);
        mAssignedModel.setCharacter(id);
    }

    QString ContractWidget::getStatusFilterButtonText(const ContractFilterProxyModel::StatusFilters &filter)
    {
        QStringList filters;
        if (filter & ContractFilterProxyModel::Outstanding)
            filters << tr("O");
        if (filter & ContractFilterProxyModel::Deleted)
            filters << tr("D");
        if (filter & ContractFilterProxyModel::Completed)
            filters << tr("C");
        if (filter & ContractFilterProxyModel::Failed)
            filters << tr("F");
        if (filter & ContractFilterProxyModel::CompletedByIssuer)
            filters << tr("Ci");
        if (filter & ContractFilterProxyModel::CompletedByContractor)
            filters << tr("Cc");
        if (filter & ContractFilterProxyModel::Cancelled)
            filters << tr("Ca");
        if (filter & ContractFilterProxyModel::Rejected)
            filters << tr("Rj");
        if (filter & ContractFilterProxyModel::Reversed)
            filters << tr("Re");
        if (filter & ContractFilterProxyModel::InProgress)
            filters << tr("Ip");

        return (filters.isEmpty()) ? (tr("Status filter")) : (tr("Status filter [%1]  ").arg(filters.join(", ")));
    }
}
