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
#include <QKeySequenceEdit>
#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QSettings>
#include <QGroupBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QLabel>

#include "PriceSettings.h"
#include "UISettings.h"

#include "PricePreferencesWidget.h"

namespace Evernus
{
    PricePreferencesWidget::PricePreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto marginGroup = new QGroupBox{tr("Margins"), this};
        mainLayout->addWidget(marginGroup);

        auto marginLayout = new QVBoxLayout{marginGroup};

        auto marginControlsLayout = new QGridLayout{};
        marginLayout->addLayout(marginControlsLayout);

        marginControlsLayout->addWidget(new QLabel{tr("Minimum:"), this}, 0, 0, Qt::AlignRight);

        mMinMarginEdit = new QDoubleSpinBox{this};
        marginControlsLayout->addWidget(mMinMarginEdit, 0, 1);
        mMinMarginEdit->setSuffix(locale().percent());
        mMinMarginEdit->setValue(settings.value(PriceSettings::minMarginKey, PriceSettings::minMarginDefault).toDouble());

        marginControlsLayout->addWidget(new QLabel{tr("Preferred:"), this}, 0, 2, Qt::AlignRight);

        mPreferredMarginEdit = new QDoubleSpinBox{this};
        marginControlsLayout->addWidget(mPreferredMarginEdit, 0, 3);
        mPreferredMarginEdit->setSuffix(locale().percent());
        mPreferredMarginEdit->setValue(settings.value(PriceSettings::preferredMarginKey, PriceSettings::preferredMarginDefault).toDouble());

        marginControlsLayout->addWidget(new QLabel{tr("Import log wait timer:"), this}, 1, 0, Qt::AlignRight);

        mImportLogWaitTimeEdit = new QSpinBox{this};
        marginControlsLayout->addWidget(mImportLogWaitTimeEdit, 1, 1);
        mImportLogWaitTimeEdit->setMaximum(10000);
        mImportLogWaitTimeEdit->setSuffix("ms");
        mImportLogWaitTimeEdit->setValue(settings.value(PriceSettings::importLogWaitTimeKey, PriceSettings::importLogWaitTimeDefault).toUInt());

        mIgnoreMinVolumeOrdersBtn = new QCheckBox{tr("Ignore orders with min. volume > 1"), this};
        marginLayout->addWidget(mIgnoreMinVolumeOrdersBtn);
        mIgnoreMinVolumeOrdersBtn->setChecked(settings.value(
            PriceSettings::ignoreOrdersWithMinVolumeKey, PriceSettings::ignoreOrdersWithMinVolumeDefault).toBool());

#ifdef Q_OS_WIN
        mAltImportBtn = new QCheckBox{tr("Use alternative margin import method*"), this};
        marginLayout->addWidget(mAltImportBtn);
        mAltImportBtn->setChecked(settings.value(PriceSettings::priceAltImportKey, PriceSettings::priceAltImportDefault).toBool());

        auto infoLabel = new QLabel{tr("* Gives faster results, but can sometimes be incorrect. If the price fluctuates after a few imports, turn it off."), this};
        infoLabel->setWordWrap(true);
        marginLayout->addWidget(infoLabel);
#endif

        auto costsGroup = new QGroupBox{tr("Costs"), this};
        mainLayout->addWidget(costsGroup);

        auto costsLayout = new QVBoxLayout{costsGroup};

        mAutoAddCustomCostBtn = new QCheckBox{tr("Auto add custom item costs on fulfilled buy order*"), this};
        costsLayout->addWidget(mAutoAddCustomCostBtn);
        mAutoAddCustomCostBtn->setChecked(settings.value(PriceSettings::autoAddCustomItemCostKey, PriceSettings::autoAddCustomItemCostDefault).toBool());

        mShareCustomCostsBtn = new QCheckBox{tr("Share costs between characters"), this};
        costsLayout->addWidget(mShareCustomCostsBtn);
        mShareCustomCostsBtn->setChecked(settings.value(PriceSettings::shareCostsKey, PriceSettings::shareCostsDefault).toBool());

        auto autoCostsLabel = new QLabel{tr("* Also turns on importing wallet transactions."), this};
        costsLayout->addWidget(autoCostsLabel);
        autoCostsLabel->setWordWrap(true);

        auto pricesGroup = new QGroupBox{this};
        mainLayout->addWidget(pricesGroup);

        auto pricesLayout = new QFormLayout{pricesGroup};

        auto priceDeltaLayout = new QHBoxLayout{};

        mPriceDeltaEdit = new QDoubleSpinBox{this};
        priceDeltaLayout->addWidget(mPriceDeltaEdit);
		mPriceDeltaEdit->setEnabled(false);
        mPriceDeltaEdit->setSingleStep(0.01);
        mPriceDeltaEdit->setMinimum(0.01);
        mPriceDeltaEdit->setMaximum(100000000.);
        mPriceDeltaEdit->setSuffix(" ISK");
        mPriceDeltaEdit->setValue(settings.value(PriceSettings::priceDeltaKey, PriceSettings::priceDeltaDefault).toDouble());

        priceDeltaLayout->addWidget(new QLabel{tr("Add random:")});

        mPriceDeltaRandomEdit = new QDoubleSpinBox{this};
        priceDeltaLayout->addWidget(mPriceDeltaRandomEdit);
		mPriceDeltaRandomEdit->setEnabled(false);
        mPriceDeltaRandomEdit->setSingleStep(0.01);
        mPriceDeltaRandomEdit->setMaximum(100000000.);
        mPriceDeltaRandomEdit->setSuffix(" ISK");
        mPriceDeltaRandomEdit->setValue(settings.value(PriceSettings::priceDeltaRandomKey, PriceSettings::priceDeltaRandomDefault).toDouble());

        pricesLayout->addRow(tr("Price delta:"), priceDeltaLayout);

        mPriceMaxAgeEdit = new QSpinBox{this};
        pricesLayout->addRow(tr("Max. price age:"), mPriceMaxAgeEdit);
        mPriceMaxAgeEdit->setMinimum(1);
        mPriceMaxAgeEdit->setMaximum(24 * 30);
        mPriceMaxAgeEdit->setSuffix("h");
        mPriceMaxAgeEdit->setValue(settings.value(PriceSettings::priceMaxAgeKey, PriceSettings::priceMaxAgeDefault).toUInt());

        mRefreshPricesWithOrdersBtn = new QCheckBox{tr("Refresh prices after order import"), this};
        pricesLayout->addRow(mRefreshPricesWithOrdersBtn);
        mRefreshPricesWithOrdersBtn->setChecked(settings.value(PriceSettings::refreshPricesWithOrdersKey).toBool());

        mCopyNonOverbidBtn = new QCheckBox{tr("Auto-copy non-overbid prices with price helper"), this};
        pricesLayout->addRow(mCopyNonOverbidBtn);
        mCopyNonOverbidBtn->setChecked(settings.value(PriceSettings::copyNonOverbidPriceKey, PriceSettings::copyNonOverbidPriceDefault).toBool());

        mLimitSellCopyToCostBtn = new QCheckBox{tr("Limit sell price copy to item cost"), this};
        pricesLayout->addRow(mLimitSellCopyToCostBtn);
        mLimitSellCopyToCostBtn->setChecked(
            settings.value(PriceSettings::limitSellCopyToCostKey, PriceSettings::limitSellCopyToCostDefault).toBool());

        auto limitCopyLayout = new QHBoxLayout{};
        pricesLayout->addRow(limitCopyLayout);

        auto limitCopyLayoutMargins = limitCopyLayout->contentsMargins();
        limitCopyLayoutMargins.setLeft(limitCopyLayoutMargins.left() + 20);
        limitCopyLayout->setContentsMargins(limitCopyLayoutMargins);

        mLimitSellCopyToTotalCostBtn = new QCheckBox{tr("Use total item costs"), this};
        limitCopyLayout->addWidget(mLimitSellCopyToTotalCostBtn);
        mLimitSellCopyToTotalCostBtn->setChecked(
            settings.value(PriceSettings::limitSellCopyToTotalCostKey, PriceSettings::limitSellCopyToTotalCostDefault).toBool());
        mLimitSellCopyToTotalCostBtn->setEnabled(mLimitSellCopyToCostBtn->isChecked());
        connect(mLimitSellCopyToCostBtn, &QCheckBox::stateChanged, mLimitSellCopyToTotalCostBtn, &QCheckBox::setEnabled);

        auto fpcGroup = new QGroupBox{tr("Fast Price Copy"), this};
        mainLayout->addWidget(fpcGroup);

        auto fpcLayout = new QVBoxLayout{fpcGroup};

        auto fpcSettingsLayout = new QHBoxLayout{};
        fpcLayout->addLayout(fpcSettingsLayout);

        mFPCBtn = new QCheckBox{tr("Enabled"), this};
        fpcSettingsLayout->addWidget(mFPCBtn);
        mFPCBtn->setChecked(settings.value(PriceSettings::fpcKey, PriceSettings::fpcDefault).toBool());

        fpcSettingsLayout->addWidget(new QLabel{tr("Forward shortcut:"), this}, 0, Qt::AlignBaseline | Qt::AlignRight);

        mFPCForwardShortcutEdit = new QKeySequenceEdit{QKeySequence::fromString(
            settings.value(PriceSettings::fpcForwardShortcutKey).toString()), this};
        fpcSettingsLayout->addWidget(mFPCForwardShortcutEdit);
        connect(mFPCForwardShortcutEdit, &QKeySequenceEdit::keySequenceChanged, this, [=](const auto &keySequence) {
            if (keySequence.matches(Qt::Key_Escape) == QKeySequence::ExactMatch)
                mFPCForwardShortcutEdit->clear();
        });

        fpcSettingsLayout->addWidget(new QLabel{tr("Backward shortcut:"), this}, 0, Qt::AlignBaseline | Qt::AlignRight);

        mFPCBackwardShortcutEdit = new QKeySequenceEdit{QKeySequence::fromString(
            settings.value(PriceSettings::fpcBackwardShortcutKey).toString()), this};
        fpcSettingsLayout->addWidget(mFPCBackwardShortcutEdit);
        connect(mFPCBackwardShortcutEdit, &QKeySequenceEdit::keySequenceChanged, this, [=](const auto &keySequence) {
            if (keySequence.matches(Qt::Key_Escape) == QKeySequence::ExactMatch)
                mFPCBackwardShortcutEdit->clear();
        });

        mShowInEveOnFPCBtn = new QCheckBox{tr("Show in EVE on copy"), this};
        fpcLayout->addWidget(mShowInEveOnFPCBtn);
        mShowInEveOnFPCBtn->setChecked(settings.value(PriceSettings::showInEveOnFpcKey, PriceSettings::showInEveOnFpcDefault).toBool());

        auto helpLabel = new QLabel{tr(
            "Fast Price Copy allows you to update your orders in a very fast manner. Simply assign a keyboard shortcut, select an order in any market order view and press the shortcut to "
            "copy the updated price and automatically jump to the next order on the list. You can do this even when Evernus doesn't have the input focus - the keyboard shortcut works system-wide. "
            "You can use this to update your prices without ever leaving Eve client."
        ), this};
        helpLabel->setWordWrap(true);

        fpcLayout->addWidget(helpLabel);

        mainLayout->addStretch();
    }

    void PricePreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(PriceSettings::minMarginKey, mMinMarginEdit->value());
        settings.setValue(PriceSettings::preferredMarginKey, mPreferredMarginEdit->value());
        settings.setValue(PriceSettings::priceDeltaKey, mPriceDeltaEdit->value());
        settings.setValue(PriceSettings::priceDeltaRandomKey, mPriceDeltaRandomEdit->value());
        settings.setValue(PriceSettings::autoAddCustomItemCostKey, mAutoAddCustomCostBtn->isChecked());
        settings.setValue(PriceSettings::shareCostsKey, mShareCustomCostsBtn->isChecked());
        settings.setValue(PriceSettings::ignoreOrdersWithMinVolumeKey, mIgnoreMinVolumeOrdersBtn->isChecked());
#ifdef Q_OS_WIN
        settings.setValue(PriceSettings::priceAltImportKey, mAltImportBtn->isChecked());
#endif
        settings.setValue(PriceSettings::importLogWaitTimeKey, mImportLogWaitTimeEdit->value());
        settings.setValue(PriceSettings::priceMaxAgeKey, mPriceMaxAgeEdit->value());
        settings.setValue(PriceSettings::refreshPricesWithOrdersKey, mRefreshPricesWithOrdersBtn->isChecked());
        settings.setValue(PriceSettings::copyNonOverbidPriceKey, mCopyNonOverbidBtn->isChecked());
        settings.setValue(PriceSettings::limitSellCopyToCostKey, mLimitSellCopyToCostBtn->isChecked());
        settings.setValue(PriceSettings::limitSellCopyToTotalCostKey, mLimitSellCopyToTotalCostBtn->isChecked());
        settings.setValue(PriceSettings::fpcKey, mFPCBtn->isChecked());
        settings.setValue(PriceSettings::fpcForwardShortcutKey, mFPCForwardShortcutEdit->keySequence().toString());
        settings.setValue(PriceSettings::fpcBackwardShortcutKey, mFPCBackwardShortcutEdit->keySequence().toString());
        settings.setValue(PriceSettings::showInEveOnFpcKey, mShowInEveOnFPCBtn->isChecked());
    }
}
