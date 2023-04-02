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
#include <QHBoxLayout>
#include <QLabel>
#include <QFont>

#include "StyledTreeView.h"
#include "ContractModel.h"
#include "TextUtils.h"

#include "ContractView.h"

namespace Evernus
{
    ContractView::ContractView(const QString &objectName, QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        mView = new StyledTreeView{objectName, this};
        mainLayout->addWidget(mView);
        mView->setModel(&mProxy);

        auto infoLayout = new QHBoxLayout{};
        mainLayout->addLayout(infoLayout);

        infoLayout->addWidget(new QLabel{tr("Total contracts:"), this});

        QFont font;
        font.setBold(true);

        mTotalContractsLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalContractsLabel);
        mTotalContractsLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total price:"), this});

        mTotalPriceLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalPriceLabel);
        mTotalPriceLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total reward:"), this});

        mTotalRewardLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalRewardLabel);
        mTotalRewardLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total collateral:"), this});

        mTotalCollateralLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalCollateralLabel);
        mTotalCollateralLabel->setFont(font);

        infoLayout->addWidget(new QLabel{tr("Total volume:"), this});

        mTotalVolumeLabel = new QLabel{"-", this};
        infoLayout->addWidget(mTotalVolumeLabel);
        mTotalVolumeLabel->setFont(font);

        infoLayout->addStretch();

        mProxy.setSortRole(Qt::UserRole);
        mProxy.setFilterCaseSensitivity(Qt::CaseInsensitive);
        connect(&mProxy, &QSortFilterProxyModel::modelReset, this, &ContractView::updateInfo);
    }

    void ContractView::setModel(ContractModel *model)
    {
        mModel = model;
        mProxy.setSourceModel(mModel);
    }

    void ContractView::setFilterWildcard(const QString &pattern)
    {
        mProxy.setFilterWildcard(pattern);
    }

    void ContractView::setStatusFilter(const ContractFilterProxyModel::StatusFilters &filter)
    {
        mProxy.setStatusFilter(filter);
    }

    void ContractView::updateInfo()
    {
        if (mModel == nullptr)
            return;

        auto curLocale = locale();

        mTotalContractsLabel->setText(curLocale.toString(static_cast<qulonglong>(mModel->getNumContracts())));
        mTotalPriceLabel->setText(TextUtils::currencyToString(mModel->getTotalPrice(), curLocale));
        mTotalRewardLabel->setText(TextUtils::currencyToString(mModel->getTotalReward(), curLocale));
        mTotalCollateralLabel->setText(TextUtils::currencyToString(mModel->getTotalCollateral(), curLocale));
        mTotalVolumeLabel->setText(QString{"%1m³"}.arg(curLocale.toString(mModel->getTotalVolume(), 'f', 2)));
    }
}
