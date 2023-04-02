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

#include "MarketOrderModel.h"
#include "FlowLayout.h"
#include "TextUtils.h"

#include "GenericMarketOrdersInfoWidget.h"

namespace Evernus
{
    GenericMarketOrdersInfoWidget::GenericMarketOrdersInfoWidget(const MarketOrderModel &model, QWidget *parent)
        : MarketOrdersInfoWidget(parent)
        , mModel(model)
    {
        QFont font;
        font.setBold(true);

        auto mainLayout = new FlowLayout{this};
        mainLayout->setContentsMargins(QMargins{});

        mainLayout->addWidget(new QLabel{tr("Active orders:"), this});

        mTotalOrdersLabel = new QLabel{this};
        mainLayout->addWidget(mTotalOrdersLabel);
        mTotalOrdersLabel->setFont(font);

        mainLayout->addWidget(new QLabel{tr("Total volume:"), this});

        mVolumeLabel = new QLabel{this};
        mainLayout->addWidget(mVolumeLabel);
        mVolumeLabel->setFont(font);

        mainLayout->addWidget(new QLabel{tr("Total ISK in orders:"), this});

        mTotalISKLabel = new QLabel{this};
        mainLayout->addWidget(mTotalISKLabel);
        mTotalISKLabel->setFont(font);

        mainLayout->addWidget(new QLabel{tr("Total size:"), this});

        mTotalSizeLabel = new QLabel{this};
        mainLayout->addWidget(mTotalSizeLabel);
        mTotalSizeLabel->setFont(font);
    }

    void GenericMarketOrdersInfoWidget::updateData()
    {
        const auto volRemaining = mModel.getVolumeRemaining();
        const auto volEntered = mModel.getVolumeEntered();

        const auto curLocale = locale();

        mTotalOrdersLabel->setText(curLocale.toString(static_cast<qulonglong>(mModel.getOrderCount())));
        mVolumeLabel->setText(QString{"%1/%2 (%3%)"}
            .arg(curLocale.toString(volRemaining))
            .arg(curLocale.toString(volEntered))
            .arg(curLocale.toString((volEntered > 0.) ? (volRemaining * 100. / volEntered) : (0.), 'f', 1)));
        mTotalISKLabel->setText(TextUtils::currencyToString(mModel.getTotalISK(), curLocale));
        mTotalSizeLabel->setText(QString{"%1m³"}.arg(curLocale.toString(mModel.getTotalSize(), 'f', 2)));
    }
}
