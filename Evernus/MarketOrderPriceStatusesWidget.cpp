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
#include <QVBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QSettings>

#include "UISettings.h"

#include "MarketOrderPriceStatusesWidget.h"

namespace Evernus
{
    const char * const MarketOrderPriceStatusesWidget::filterPropertyName = "filter";

    MarketOrderPriceStatusesWidget::MarketOrderPriceStatusesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;
        mCurrentFilter = static_cast<MarketOrderFilterProxyModel::PriceStatusFilters>(
            settings.value(UISettings::marketOrderPriceStatusFilterKey, static_cast<int>(MarketOrderFilterProxyModel::defaultPriceStatusFilter)).toInt());

        auto mainLayout = new QVBoxLayout{this};

        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Ok, tr("Ok")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::NoData, tr("No data")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::DataTooOld, tr("Data too old")));

        auto checkAllBtn = new QPushButton{tr("Check all"), this};
        mainLayout->addWidget(checkAllBtn);
        connect(checkAllBtn, &QPushButton::clicked, this, &MarketOrderPriceStatusesWidget::checkAll);
    }

    MarketOrderFilterProxyModel::PriceStatusFilters MarketOrderPriceStatusesWidget::getStatusFilter() const noexcept
    {
        return mCurrentFilter;
    }

    void MarketOrderPriceStatusesWidget::changeFilter(int state)
    {
        const auto flag = static_cast<MarketOrderFilterProxyModel::PriceStatusFilter>(sender()->property(filterPropertyName).toInt());

        if (state == Qt::Checked)
            mCurrentFilter |= flag;
        else
            mCurrentFilter &= ~flag;

        setNewFilter(mCurrentFilter);
    }

    void MarketOrderPriceStatusesWidget::checkAll()
    {
        const auto boxes = findChildren<QCheckBox *>();
        for (auto box : boxes)
        {
            box->blockSignals(true);
            box->setChecked(true);
            box->blockSignals(false);
        }

        setNewFilter(MarketOrderFilterProxyModel::EveryPriceStatus);
    }

    QCheckBox *MarketOrderPriceStatusesWidget::createCheckBox(MarketOrderFilterProxyModel::PriceStatusFilter filter, const QString &label)
    {
        auto checkBtn = new QCheckBox{label, this};
        checkBtn->setProperty(filterPropertyName, filter);
        checkBtn->setChecked(mCurrentFilter & filter);
        connect(checkBtn, &QCheckBox::stateChanged, this, &MarketOrderPriceStatusesWidget::changeFilter);

        return checkBtn;
    }

    void MarketOrderPriceStatusesWidget::setNewFilter(const MarketOrderFilterProxyModel::PriceStatusFilters &filter)
    {
        mCurrentFilter = filter;

        QSettings settings;
        settings.setValue(UISettings::marketOrderPriceStatusFilterKey, static_cast<int>(mCurrentFilter));

        emit filterChanged(mCurrentFilter);
    }
}
