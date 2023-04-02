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
#include <QHBoxLayout>
#include <QPushButton>
#include <QDateEdit>
#include <QSettings>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

#include "MarketAnalysisSettings.h"

#include "TypeAggregatedDetailsFilterWidget.h"

namespace Evernus
{
    TypeAggregatedDetailsFilterWidget::TypeAggregatedDetailsFilterWidget(QWidget *parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
    {
        auto mainLayout = new QHBoxLayout{this};

        mainLayout->addWidget(new QLabel{tr("From:"), this});

        const auto current = QDate::currentDate().addDays(-1);

        QSettings settings;

        mFromEdit = new QDateEdit{this};
        mainLayout->addWidget(mFromEdit);
        mFromEdit->setCalendarPopup(true);
        mFromEdit->setDate(current.addDays(
            -settings.value(MarketAnalysisSettings::typeAggregatedChartDurationKey, MarketAnalysisSettings::typeAggregatedChartDurationDefault).toInt()));
        mFromEdit->setMaximumDate(current);
        connect(mFromEdit, &QDateEdit::dateChanged, this, [=](const QDate &date) {
            if (date > mToEdit->date())
                mToEdit->setDate(date);
        });

        mainLayout->addWidget(new QLabel{tr("To:"), this});

        mToEdit = new QDateEdit{this};
        mainLayout->addWidget(mToEdit);
        mToEdit->setCalendarPopup(true);
        mToEdit->setDate(current);
        mToEdit->setMaximumDate(current);
        connect(mToEdit, &QDateEdit::dateChanged, this, [=](const QDate &date) {
            if (date < mFromEdit->date())
                mFromEdit->setDate(date);
        });

        mainLayout->addWidget(new QLabel{tr("Moving average days:"), this});

        mSMADaysEdit = new QSpinBox{this};
        mainLayout->addWidget(mSMADaysEdit);
        mSMADaysEdit->setMinimum(2);
        mSMADaysEdit->setValue(settings.value(MarketAnalysisSettings::smaDaysKey, MarketAnalysisSettings::smaDaysDefault).toInt());

        mainLayout->addWidget(new QLabel{tr("MACD days:"), this});

        mMACDFastDaysEdit = new QSpinBox{this};
        mainLayout->addWidget(mMACDFastDaysEdit);
        mMACDFastDaysEdit->setToolTip(tr("Fast days."));
        mMACDFastDaysEdit->setMinimum(2);
        mMACDFastDaysEdit->setValue(settings.value(MarketAnalysisSettings::macdFastDaysKey, MarketAnalysisSettings::macdFastDaysDefault).toInt());

        mMACDSlowDaysEdit = new QSpinBox{this};
        mainLayout->addWidget(mMACDSlowDaysEdit);
        mMACDSlowDaysEdit->setToolTip(tr("Slow days."));
        mMACDSlowDaysEdit->setMinimum(2);
        mMACDSlowDaysEdit->setValue(settings.value(MarketAnalysisSettings::macdSlowDaysKey, MarketAnalysisSettings::macdSlowDaysDefault).toInt());

        mMACDEMADaysEdit = new QSpinBox{this};
        mainLayout->addWidget(mMACDEMADaysEdit);
        mMACDEMADaysEdit->setToolTip(tr("EMA days."));
        mMACDEMADaysEdit->setMinimum(2);
        mMACDEMADaysEdit->setValue(settings.value(MarketAnalysisSettings::macdEmaDaysKey, MarketAnalysisSettings::macdEmaDaysDefault).toInt());

        mainLayout->addWidget(new QLabel{tr("Volume graph:"), this});

        mVolumeTypeEdit = new QComboBox{this};
        mainLayout->addWidget(mVolumeTypeEdit);
        mVolumeTypeEdit->addItem(tr("Volume"), static_cast<int>(VolumeType::Volume));
        mVolumeTypeEdit->addItem(tr("Order count"), static_cast<int>(VolumeType::OrderCount));
        mVolumeTypeEdit->setCurrentIndex(mVolumeTypeEdit->findData(
            settings.value(MarketAnalysisSettings::volumeGraphTypeKey, MarketAnalysisSettings::volumeGraphTypeDefault).toInt()
        ));

        auto filterBtn = new QPushButton{tr("Apply"), this};
        mainLayout->addWidget(filterBtn);
        connect(filterBtn, &QPushButton::clicked, this, [=] {
            const auto smaDays = getSMADays();
            const auto macdFastDays = getMACDFastDays();
            const auto macdSlowDays = getMACDSlowDays();
            const auto macdEmaDays = getMACDEMADays();
            const auto volumeType = getVolumeType();

            QSettings settings;
            settings.setValue(MarketAnalysisSettings::smaDaysKey, smaDays);
            settings.setValue(MarketAnalysisSettings::macdFastDaysKey, macdFastDays);
            settings.setValue(MarketAnalysisSettings::macdSlowDaysKey, macdSlowDays);
            settings.setValue(MarketAnalysisSettings::macdEmaDaysKey, macdEmaDays);
            settings.setValue(MarketAnalysisSettings::volumeGraphTypeKey,
                              static_cast<int>(volumeType));

            emit applyFilter(getFrom(),
                             getTo(),
                             smaDays,
                             macdFastDays,
                             macdSlowDays,
                             macdEmaDays,
                             volumeType);
        });

        auto addTrendLineBtn = new QPushButton{tr("Add trend line"), this};
        mainLayout->addWidget(addTrendLineBtn);
        connect(addTrendLineBtn, &QPushButton::clicked, this, [=] {
            emit addTrendLine(getFrom(), getTo());
        });

        const auto showLegend
            = settings.value(MarketAnalysisSettings::showLegendKey, MarketAnalysisSettings::showLegendDefault).toBool();

        auto legendBtn = new QCheckBox{tr("Show legend"), this};
        mainLayout->addWidget(legendBtn);
        legendBtn->setChecked(showLegend);
        connect(legendBtn, &QCheckBox::stateChanged, this, &TypeAggregatedDetailsFilterWidget::showLegend);

        mainLayout->addStretch();
    }

    QDate TypeAggregatedDetailsFilterWidget::getFrom() const
    {
        return mFromEdit->date();
    }

    QDate TypeAggregatedDetailsFilterWidget::getTo() const
    {
        return mToEdit->date();
    }

    int TypeAggregatedDetailsFilterWidget::getSMADays() const
    {
        return mSMADaysEdit->value();
    }

    int TypeAggregatedDetailsFilterWidget::getMACDFastDays() const
    {
        return mMACDFastDaysEdit->value();
    }

    int TypeAggregatedDetailsFilterWidget::getMACDSlowDays() const
    {
        return mMACDSlowDaysEdit->value();
    }

    int TypeAggregatedDetailsFilterWidget::getMACDEMADays() const
    {
        return mMACDEMADaysEdit->value();
    }

    VolumeType TypeAggregatedDetailsFilterWidget::getVolumeType() const
    {
        return static_cast<VolumeType>(mVolumeTypeEdit->currentData().toInt());
    }
}
