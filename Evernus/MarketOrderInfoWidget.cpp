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
#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QClipboard>
#include <QCheckBox>
#include <QSettings>
#include <QEvent>
#include <QLabel>

#include "UISettings.h"
#include "TextUtils.h"

#include "MarketOrderInfoWidget.h"
#include "PriceUtils.h"

namespace Evernus
{
    MarketOrderInfoWidget::MarketOrderInfoWidget(const MarketOrderModel::OrderInfo &info, QWidget *parent)
        : QFrame(parent, Qt::Tool | Qt::FramelessWindowHint)
        , mTargetPrice(QString::number(info.mTargetPrice, 'f', 2))
#ifdef Q_OS_MAC
        , mWasDeactivated(false)
#endif
    {
        auto mainLayout = new QVBoxLayout{this};
        mainLayout->setContentsMargins(3, 3, 3, 3);

        auto btnLayout = new QHBoxLayout{};
        mainLayout->addLayout(btnLayout);

        auto closeBtn = new QPushButton{QIcon{":/images/cross.png"}, tr("Close"), this};
        btnLayout->addWidget(closeBtn);
        closeBtn->setFlat(true);
        connect(closeBtn, &QPushButton::clicked, this, &MarketOrderInfoWidget::deleteLater);

        auto copyBtn = new QPushButton{QIcon{":/images/page_copy.png"}, tr("Copy"), this};
        btnLayout->addWidget(copyBtn);
        copyBtn->setFlat(true);
        connect(copyBtn, &QPushButton::clicked, this, &MarketOrderInfoWidget::copyPrice);

        QSettings settings;

        const auto autoCopy
            = settings.value(UISettings::autoCopyPriceFromInfoKey, UISettings::autoCopyPriceFromInfoDefault).toBool();

        auto copyCheck = new QCheckBox{tr("Copy new price on open"), this};
        btnLayout->addWidget(copyCheck);
        copyCheck->setChecked(autoCopy);
        connect(copyCheck, &QCheckBox::stateChanged, this, &MarketOrderInfoWidget::setAutoCopy);

        btnLayout->addStretch();

        auto infoLayout = new QGridLayout{};
        mainLayout->addLayout(infoLayout);

        const auto curLocale = locale();

        infoLayout->addWidget(new QLabel{tr("<span style='color: green'>Your price:</span>"), this}, 0, 0);
        infoLayout->addWidget(new QLabel{tr("<span style='color: green'>%1</span>")
            .arg(TextUtils::currencyToString(info.mOrderPrice, curLocale)), this}, 0, 1);
        infoLayout->addWidget(new QLabel{tr("Valid on:"), this}, 0, 2);
        infoLayout->addWidget(new QLabel{(info.mOrderLocalTimestamp.isValid()) ? (TextUtils::dateTimeToString(info.mOrderLocalTimestamp, curLocale)) : ("-"), this}, 0, 3);

        infoLayout->addWidget(new QLabel{tr("<span style='color: red'>Market price:</span>"), this}, 1, 0);
        infoLayout->addWidget(new QLabel{tr("<span style='color: red'>%1</span>")
            .arg(TextUtils::currencyToString(info.mMarketPrice, curLocale)), this}, 1, 1);
        infoLayout->addWidget(new QLabel{tr("Valid on:"), this}, 1, 2);
        infoLayout->addWidget(new QLabel{(info.mMarketLocalTimestamp.isValid()) ? (TextUtils::dateTimeToString(info.mMarketLocalTimestamp, curLocale)) : ("-"), this}, 1, 3);

        infoLayout->addWidget(new QLabel{tr("Difference:"), this}, 2, 0);
        infoLayout->addWidget(new QLabel{TextUtils::currencyToString(info.mMarketPrice - info.mOrderPrice, curLocale), this}, 2, 1);
        infoLayout->addWidget(new QLabel{tr("New price:"), this}, 2, 2);
        infoLayout->addWidget(new QLabel{QString{"<strong>%1</strong>"}.arg(TextUtils::currencyToString( info.mTargetPrice, curLocale)), this}, 2, 3);

        setFrameStyle(QFrame::StyledPanel);

        if (autoCopy)
            QApplication::clipboard()->setText(mTargetPrice);
    }

    void MarketOrderInfoWidget::setAutoCopy(int state)
    {
        QSettings settings;
        settings.setValue(UISettings::autoCopyPriceFromInfoKey, state == Qt::Checked);
    }

    void MarketOrderInfoWidget::copyPrice()
    {
        QApplication::clipboard()->setText(mTargetPrice);
    }

    bool MarketOrderInfoWidget::event(QEvent *event)
    {
        const auto type = event->type();
        if (type == QEvent::WindowDeactivate || type == QEvent::FocusOut)
        {
#ifdef Q_OS_MAC
            if (!mWasDeactivated) {
                mWasDeactivated = true;
                return QFrame::event(event);
            }
#endif
            event->accept();
            deleteLater();
            return true;
        }

        return QFrame::event(event);
    }
}
