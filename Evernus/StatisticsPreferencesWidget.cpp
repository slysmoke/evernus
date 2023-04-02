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
#include <QFormLayout>
#include <QSettings>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>

#include "MarketAnalysisSettings.h"
#include "StatisticsSettings.h"
#include "ColorButton.h"
#include "UISettings.h"

#include "StatisticsPreferencesWidget.h"

namespace Evernus
{
    StatisticsPreferencesWidget::StatisticsPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto mainGroup = new QGroupBox{this};
        mainLayout->addWidget(mainGroup);

        auto mainGroupLayout = new QVBoxLayout{mainGroup};

        QSettings settings;

        mCombineCorpAndCharPlotsBtn = new QCheckBox{tr("Combine character and corporation journal in statistics"), this};
        mainGroupLayout->addWidget(mCombineCorpAndCharPlotsBtn);
        mCombineCorpAndCharPlotsBtn->setChecked(
            settings.value(StatisticsSettings::combineCorpAndCharPlotsKey, StatisticsSettings::combineCorpAndCharPlotsDefault).toBool());

        mAutomaticSnapshotsBtn = new QCheckBox{tr("Take automatic value snapshots"), this};
        mainGroupLayout->addWidget(mAutomaticSnapshotsBtn);
        mAutomaticSnapshotsBtn->setChecked(
            settings.value(StatisticsSettings::automaticSnapshotsKey, StatisticsSettings::automaticSnapshotsDefault).toBool());

        auto appearanceGroup = new QGroupBox{tr("Appearance"), this};
        mainLayout->addWidget(appearanceGroup);

        auto appearanceGroupLayout = new QFormLayout{appearanceGroup};

        const auto plotNumberFormat
            = settings.value(UISettings::plotNumberFormatKey, UISettings::plotNumberFormatDefault).toString();

        mPlotNumberFormatEdit = new QComboBox{this};
        appearanceGroupLayout->addRow(tr("Plot number format:"), mPlotNumberFormatEdit);
        addPlotFormat(tr("beautified scientific"), "gbc", plotNumberFormat);
        addPlotFormat(tr("scientific"), "g", plotNumberFormat);
        addPlotFormat(tr("fixed"), "f", plotNumberFormat);

        mAssetPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsAssetPlotColorKey, StatisticsSettings::statisticsAssetPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Asset value plot color:"), mAssetPlotColorBtn);
        mCorpAssetPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsCorpAssetPlotColorKey, StatisticsSettings::statisticsCorpAssetPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Corp. asset value plot color:"), mCorpAssetPlotColorBtn);
        mWalletPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsWalletPlotColorKey, StatisticsSettings::statisticsWalletPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Wallet balance plot color:"), mWalletPlotColorBtn);
        mCorpWalletPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsCorpWalletPlotColorKey, StatisticsSettings::statisticsCorpWalletPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Corp. wallet balance plot color:"), mCorpWalletPlotColorBtn);
        mBuyOrdersPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsBuyOrderPlotColorKey, StatisticsSettings::statisticsBuyOrderPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Buy order value plot color:"), mBuyOrdersPlotColorBtn);
        mSellOrdersPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsSellOrderPlotColorKey, StatisticsSettings::statisticsSellOrderPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Sell order value plot color:"), mSellOrdersPlotColorBtn);
        mTotalPlotColorBtn = new ColorButton{
            settings.value(StatisticsSettings::statisticsTotalPlotColorKey, StatisticsSettings::statisticsTotalPlotColorDefault).value<QColor>(),
            this
        };
        appearanceGroupLayout->addRow(tr("Total value plot color:"), mTotalPlotColorBtn);

        auto resetColors = new QPushButton(tr("Reset to default"), this);
        appearanceGroupLayout->addRow(resetColors);
        connect(resetColors, &QPushButton::clicked, this, &StatisticsPreferencesWidget::resetPlotColors);

        const auto miscGroup = new QGroupBox{this};
        mainLayout->addWidget(miscGroup);

        const auto miscGroupLayout = new QFormLayout{miscGroup};

        mDefaultAggregatedGraphPeriodEdit = new QSpinBox{this};
        miscGroupLayout->addRow(tr("Default period for item details graphs:"), mDefaultAggregatedGraphPeriodEdit);
        mDefaultAggregatedGraphPeriodEdit->setRange(1, 365);
        mDefaultAggregatedGraphPeriodEdit->setSuffix(tr(" days"));
        mDefaultAggregatedGraphPeriodEdit->setValue(
            settings.value(MarketAnalysisSettings::typeAggregatedChartDurationKey, MarketAnalysisSettings::typeAggregatedChartDurationDefault).toInt());

        mainLayout->addStretch();
    }

    void StatisticsPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(StatisticsSettings::combineCorpAndCharPlotsKey, mCombineCorpAndCharPlotsBtn->isChecked());
        settings.setValue(StatisticsSettings::automaticSnapshotsKey, mAutomaticSnapshotsBtn->isChecked());
        settings.setValue(UISettings::plotNumberFormatKey, mPlotNumberFormatEdit->currentData());
        settings.setValue(StatisticsSettings::statisticsAssetPlotColorKey, mAssetPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsCorpAssetPlotColorKey, mCorpAssetPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsWalletPlotColorKey, mWalletPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsCorpWalletPlotColorKey, mCorpWalletPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsBuyOrderPlotColorKey, mBuyOrdersPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsSellOrderPlotColorKey, mSellOrdersPlotColorBtn->getColor());
        settings.setValue(StatisticsSettings::statisticsTotalPlotColorKey, mTotalPlotColorBtn->getColor());
        settings.setValue(MarketAnalysisSettings::typeAggregatedChartDurationKey, mDefaultAggregatedGraphPeriodEdit->value());
    }

    void StatisticsPreferencesWidget::resetPlotColors()
    {
        mAssetPlotColorBtn->setColor(StatisticsSettings::statisticsAssetPlotColorDefault);
        mWalletPlotColorBtn->setColor(StatisticsSettings::statisticsWalletPlotColorDefault);
        mCorpWalletPlotColorBtn->setColor(StatisticsSettings::statisticsCorpWalletPlotColorDefault);
        mBuyOrdersPlotColorBtn->setColor(StatisticsSettings::statisticsBuyOrderPlotColorDefault);
        mSellOrdersPlotColorBtn->setColor(StatisticsSettings::statisticsSellOrderPlotColorDefault);
        mTotalPlotColorBtn->setColor(StatisticsSettings::statisticsTotalPlotColorDefault);
    }

    void StatisticsPreferencesWidget::addPlotFormat(const QString &text, const QString &value, const QString &curValue)
    {
        mPlotNumberFormatEdit->addItem(text, value);
        if (curValue == value)
            mPlotNumberFormatEdit->setCurrentIndex(mPlotNumberFormatEdit->count() - 1);
    }
}
