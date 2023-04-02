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
#include <QFont>

#include "UISettings.h"

#include "MarketOrderStatesWidget.h"

namespace Evernus
{
    const char * const MarketOrderStatesWidget::filterPropertyName = "filter";

    MarketOrderStatesWidget::MarketOrderStatesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;
        mCurrentFilter = static_cast<MarketOrderFilterProxyModel::StatusFilters>(
            settings.value(UISettings::marketOrderStateFilterKey, static_cast<int>(MarketOrderFilterProxyModel::defaultStatusFilter)).toInt());

        auto mainLayout = new QVBoxLayout{this};

        QFont font;
        font.setBold(true);

        auto checkBtn = createCheckBox(MarketOrderFilterProxyModel::Changed, tr("Changed"));
        mainLayout->addWidget(checkBtn);
        checkBtn->setFont(font);

        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Active, tr("Active")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Fulfilled, tr("Fulfilled")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Cancelled, tr("Cancelled")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Pending, tr("Pending")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::CharacterDeleted, tr("Deleted")));
        mainLayout->addWidget(createCheckBox(MarketOrderFilterProxyModel::Expired, tr("Expired")));

        auto checkAllBtn = new QPushButton{tr("Check all"), this};
        mainLayout->addWidget(checkAllBtn);
        connect(checkAllBtn, &QPushButton::clicked, this, &MarketOrderStatesWidget::checkAll);

        auto resetBtn = new QPushButton{tr("Reset"), this};
        mainLayout->addWidget(resetBtn);
        connect(resetBtn, &QPushButton::clicked, this, &MarketOrderStatesWidget::reset);
    }

    MarketOrderFilterProxyModel::StatusFilters MarketOrderStatesWidget::getStatusFilter() const noexcept
    {
        return mCurrentFilter;
    }

    void MarketOrderStatesWidget::changeFilter(int state)
    {
        const auto flag = static_cast<MarketOrderFilterProxyModel::StatusFilter>(sender()->property(filterPropertyName).toInt());

        if (state == Qt::Checked)
            mCurrentFilter |= flag;
        else
            mCurrentFilter &= ~flag;

        setNewFilter(mCurrentFilter);
    }

    void MarketOrderStatesWidget::checkAll()
    {
        const auto boxes = findChildren<QCheckBox *>();
        for (auto box : boxes)
        {
            box->blockSignals(true);
            box->setChecked(true);
            box->blockSignals(false);
        }

        setNewFilter(MarketOrderFilterProxyModel::EveryStatus);
    }

    void MarketOrderStatesWidget::reset()
    {
        const auto boxes = findChildren<QCheckBox *>();
        for (auto box : boxes)
        {
            const auto flag = static_cast<MarketOrderFilterProxyModel::StatusFilter>(box->property(filterPropertyName).toInt());

            box->blockSignals(true);
            box->setChecked(MarketOrderFilterProxyModel::defaultStatusFilter & flag);
            box->blockSignals(false);
        }

        setNewFilter(MarketOrderFilterProxyModel::defaultStatusFilter);
    }

    QCheckBox *MarketOrderStatesWidget::createCheckBox(MarketOrderFilterProxyModel::StatusFilter filter, const QString &label)
    {
        auto checkBtn = new QCheckBox{label, this};
        checkBtn->setProperty(filterPropertyName, filter);
        checkBtn->setChecked(mCurrentFilter & filter);
        connect(checkBtn, &QCheckBox::stateChanged, this, &MarketOrderStatesWidget::changeFilter);

        return checkBtn;
    }

    void MarketOrderStatesWidget::setNewFilter(const MarketOrderFilterProxyModel::StatusFilters &filter)
    {
        mCurrentFilter = filter;

        QSettings settings;
        settings.setValue(UISettings::marketOrderStateFilterKey, static_cast<int>(mCurrentFilter));

        emit filterChanged(mCurrentFilter);
    }
}
