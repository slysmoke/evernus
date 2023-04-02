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
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QMenu>

#include "MarketOrderPriceStatusesWidget.h"
#include "MarketOrderStatesWidget.h"
#include "TextFilterWidget.h"

#include "MarketOrderFilterWidget.h"

namespace Evernus
{
    MarketOrderFilterWidget::MarketOrderFilterWidget(const FilterTextRepository &filterRepo, QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QHBoxLayout{this};
        mainLayout->setContentsMargins(QMargins{});

        auto statesWidget = new MarketOrderStatesWidget{this};
        connect(statesWidget, &MarketOrderStatesWidget::filterChanged, this, &MarketOrderFilterWidget::setStatusFilter);

        auto priceStatusesWidget = new MarketOrderPriceStatusesWidget{this};
        connect(priceStatusesWidget, &MarketOrderPriceStatusesWidget::filterChanged, this, &MarketOrderFilterWidget::setPriceStatusFilter);

        auto filterStateAction = new QWidgetAction{this};
        filterStateAction->setDefaultWidget(statesWidget);

        auto filterPriceStatusAction = new QWidgetAction{this};
        filterPriceStatusAction->setDefaultWidget(priceStatusesWidget);

        auto filterStateMenu = new QMenu{this};
        filterStateMenu->addAction(filterStateAction);

        auto filterPriceStatusMenu = new QMenu{this};
        filterPriceStatusMenu->addAction(filterPriceStatusAction);

        mStateFilterBtn = new QPushButton{QIcon{":/images/flag_green.png"}, getStateFilterButtonText(statesWidget->getStatusFilter()), this};
        mainLayout->addWidget(mStateFilterBtn);
        mStateFilterBtn->setFlat(true);
        mStateFilterBtn->setMenu(filterStateMenu);

        mPriceStatusFilterBtn
            = new QPushButton{QIcon{":/images/flag_yellow.png"}, getPriceStatusFilterButtonText(priceStatusesWidget->getStatusFilter()), this};
        mainLayout->addWidget(mPriceStatusFilterBtn);
        mPriceStatusFilterBtn->setFlat(true);
        mPriceStatusFilterBtn->setMenu(filterPriceStatusMenu);

        mFilterEdit = new TextFilterWidget{filterRepo, this};
        mainLayout->addWidget(mFilterEdit, 1);
        connect(mFilterEdit, &TextFilterWidget::filterEntered, this, &MarketOrderFilterWidget::applyTextFilter);

        mScriptFilterBtn = new QCheckBox{tr("Script"), this};
        mainLayout->addWidget(mScriptFilterBtn);
        connect(mScriptFilterBtn, &QCheckBox::toggled, this, &MarketOrderFilterWidget::applyScriptChange);
    }

    void MarketOrderFilterWidget::setStatusFilter(const MarketOrderFilterProxyModel::StatusFilters &filter)
    {
        mStateFilterBtn->setText(getStateFilterButtonText(filter));
        emit statusFilterChanged(filter);
    }

    void MarketOrderFilterWidget::setPriceStatusFilter(const MarketOrderFilterProxyModel::PriceStatusFilters &filter)
    {
        mPriceStatusFilterBtn->setText(getPriceStatusFilterButtonText(filter));
        emit priceStatusFilterChanged(filter);
    }

    void MarketOrderFilterWidget::applyTextFilter(const QString &text)
    {
        emit textFilterChanged(text, mScriptFilterBtn->isChecked());
    }

    void MarketOrderFilterWidget::applyScriptChange(bool scriptChecked)
    {
        mFilterEdit->clearEditText();
        mFilterEdit->lineEdit()->setPlaceholderText((scriptChecked) ?
                                                    (tr("type in script and press Enter")) :
                                                    (TextFilterWidget::getDefaultPlaceholderText()));

        emit textFilterChanged(QString{}, mScriptFilterBtn->isChecked());
    }

    QString MarketOrderFilterWidget::getStateFilterButtonText(const MarketOrderFilterProxyModel::StatusFilters &filter)
    {
        QStringList filters;
        if (filter & MarketOrderFilterProxyModel::Changed)
            filters << tr("Ch");
        if (filter & MarketOrderFilterProxyModel::Active)
            filters << tr("A");
        if (filter & MarketOrderFilterProxyModel::Fulfilled)
            filters << tr("F");
        if (filter & MarketOrderFilterProxyModel::Cancelled)
            filters << tr("C");
        if (filter & MarketOrderFilterProxyModel::Pending)
            filters << tr("P");
        if (filter & MarketOrderFilterProxyModel::CharacterDeleted)
            filters << tr("D");
        if (filter & MarketOrderFilterProxyModel::Expired)
            filters << tr("E");

        return (filters.isEmpty()) ? (tr("Status filter")) : (tr("Status filter [%1]  ").arg(filters.join(", ")));
    }

    QString MarketOrderFilterWidget::getPriceStatusFilterButtonText(const MarketOrderFilterProxyModel::PriceStatusFilters &filter)
    {
        QStringList filters;
        if (filter & MarketOrderFilterProxyModel::Ok)
            filters << tr("Ok");
        if (filter & MarketOrderFilterProxyModel::NoData)
            filters << tr("N");
        if (filter & MarketOrderFilterProxyModel::DataTooOld)
            filters << tr("O");

        return (filters.isEmpty()) ? (tr("Price status filter")) : (tr("Price status filter [%1]  ").arg(filters.join(", ")));
    }
}
