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

#include "ContractStatusesWidget.h"

namespace Evernus
{
    const char * const ContractStatusesWidget::filterPropertyName = "filter";

    ContractStatusesWidget::ContractStatusesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;
        mCurrentFilter = static_cast<ContractFilterProxyModel::StatusFilters>(
            settings.value(UISettings::contractStatusFilterKey, static_cast<int>(ContractFilterProxyModel::defaultStatusFilter)).toInt());

        auto mainLayout = new QVBoxLayout{this};

        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Outstanding, tr("Outstanding")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Deleted, tr("Deleted")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Completed, tr("Completed")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Failed, tr("Failed")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::CompletedByIssuer, tr("Completed by Issuer")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::CompletedByContractor, tr("Completed by Contractor")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Cancelled, tr("Cancelled")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Rejected, tr("Rejected")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::Reversed, tr("Reversed")));
        mainLayout->addWidget(createCheckBox(ContractFilterProxyModel::InProgress, tr("In Progress")));

        auto resetBtn = new QPushButton{tr("Reset"), this};
        mainLayout->addWidget(resetBtn);
        connect(resetBtn, &QPushButton::clicked, this, &ContractStatusesWidget::reset);
    }

    ContractFilterProxyModel::StatusFilters ContractStatusesWidget::getStatusFilter() const noexcept
    {
        return mCurrentFilter;
    }

    void ContractStatusesWidget::changeFilter(int state)
    {
        const auto flag = static_cast<ContractFilterProxyModel::StatusFilter>(sender()->property(filterPropertyName).toInt());

        if (state == Qt::Checked)
            mCurrentFilter |= flag;
        else
            mCurrentFilter &= ~flag;

        setNewFilter(mCurrentFilter);
    }

    void ContractStatusesWidget::reset()
    {
        const auto boxes = findChildren<QCheckBox *>();
        for (auto box : boxes)
        {
            const auto flag = static_cast<ContractFilterProxyModel::StatusFilter>(box->property(filterPropertyName).toInt());

            box->blockSignals(true);
            box->setChecked(ContractFilterProxyModel::defaultStatusFilter & flag);
            box->blockSignals(false);
        }

        setNewFilter(ContractFilterProxyModel::defaultStatusFilter);
    }

    QCheckBox *ContractStatusesWidget::createCheckBox(ContractFilterProxyModel::StatusFilter filter, const QString &label)
    {
        auto checkBtn = new QCheckBox{label, this};
        checkBtn->setProperty(filterPropertyName, filter);
        checkBtn->setChecked(mCurrentFilter & filter);
        connect(checkBtn, &QCheckBox::stateChanged, this, &ContractStatusesWidget::changeFilter);

        return checkBtn;
    }

    void ContractStatusesWidget::setNewFilter(const ContractFilterProxyModel::StatusFilters &filter)
    {
        mCurrentFilter = filter;

        QSettings settings;
        settings.setValue(UISettings::contractStatusFilterKey, static_cast<int>(mCurrentFilter));

        emit filterChanged(mCurrentFilter);
    }
}
