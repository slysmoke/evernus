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
#include <QLabel>
#include <QFont>

#include "MarketOrderSellModel.h"
#include "FlowLayout.h"
#include "PriceUtils.h"
#include "TextUtils.h"

#include "SellMarketOrdersInfoWidget.h"

namespace Evernus
{
    SellMarketOrdersInfoWidget::SellMarketOrdersInfoWidget(const MarketOrderSellModel &model, QWidget *parent)
        : GenericMarketOrdersInfoWidget(model, parent)
        , mModel(model)
    {
        QFont font;
        font.setBold(true);

        auto mainLayout = layout();

        mainLayout->addWidget(new QLabel{tr("Total income:"), this});

        mTotalIncomeLabel = new QLabel{this};
        mainLayout->addWidget(mTotalIncomeLabel);
        mTotalIncomeLabel->setFont(font);
        mTotalIncomeLabel->setStyleSheet("color: lime;");

        mainLayout->addWidget(new QLabel{tr("Total cost:"), this});

        mTotalCostLabel = new QLabel{this};
        mainLayout->addWidget(mTotalCostLabel);
        mTotalCostLabel->setFont(font);
        mTotalCostLabel->setStyleSheet("color: red;");

        mainLayout->addWidget(new QLabel{tr("Total margin:"), this});

        mTotalMarginLabel = new QLabel{this};
        mainLayout->addWidget(mTotalMarginLabel);
        mTotalMarginLabel->setFont(font);
    }

    void SellMarketOrdersInfoWidget::updateData()
    {
        GenericMarketOrdersInfoWidget::updateData();

        const auto curLocale = locale();
        const auto income = mModel.getTotalIncome();
        const auto cost = mModel.getTotalCost();
        const auto margin = (qFuzzyIsNull(income)) ? (100.) : (100. * (income - cost) / income);

        mTotalIncomeLabel->setText(TextUtils::currencyToString(income, curLocale));
        mTotalCostLabel->setText(TextUtils::currencyToString(cost, curLocale));
        mTotalMarginLabel->setText(QString{"%1%2"}.arg(curLocale.toString(margin, 'f', 2)).arg(curLocale.percent()));

        mTotalMarginLabel->setStyleSheet(TextUtils::getMarginStyleSheet(margin));
    }
}
