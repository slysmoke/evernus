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
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QSpinBox>
#include <QLabel>

#include "StationSelectDialog.h"
#include "EveDataProvider.h"
#include "OrderSettings.h"

#include "OrderPreferencesWidget.h"

namespace Evernus
{
    OrderPreferencesWidget::OrderPreferencesWidget(const EveDataProvider &dataProvider, QWidget *parent)
        : QWidget(parent)
        , mDataProvider{dataProvider}
    {
        auto mainLayout = new QVBoxLayout{this};

        auto mainGroup = new QGroupBox{this};
        mainLayout->addWidget(mainGroup);

        auto mainGroupLayout = new QFormLayout{mainGroup};

        QSettings settings;

        mMarketOrderMaxAgeEdit = new QSpinBox{this};
        mainGroupLayout->addRow(tr("Max. market order age:"), mMarketOrderMaxAgeEdit);
        mMarketOrderMaxAgeEdit->setMinimum(1);
        mMarketOrderMaxAgeEdit->setMaximum(365);
        mMarketOrderMaxAgeEdit->setSuffix(tr(" days"));
        mMarketOrderMaxAgeEdit->setValue(settings.value(OrderSettings::marketOrderMaxAgeKey, OrderSettings::marketOrderMaxAgeDefault).toUInt());

        mOldMarketOrdersDaysEdit = new QSpinBox{this};

        mDeleteOldMarketOrdersBtn = new QCheckBox{tr("Delete old fulfilled orders"), this};
        mainGroupLayout->addRow(mDeleteOldMarketOrdersBtn);
        mDeleteOldMarketOrdersBtn->setChecked(settings.value(OrderSettings::deleteOldMarketOrdersKey, OrderSettings::deleteOldMarketOrdersDefault).toBool());
        connect(mDeleteOldMarketOrdersBtn, &QCheckBox::stateChanged, mOldMarketOrdersDaysEdit, &QSpinBox::setEnabled);

        mainGroupLayout->addRow(tr("Delete older than:"), mOldMarketOrdersDaysEdit);
        mOldMarketOrdersDaysEdit->setMinimum(14);
        mOldMarketOrdersDaysEdit->setMaximum(365 * 2);
        mOldMarketOrdersDaysEdit->setSuffix(tr(" days"));
        mOldMarketOrdersDaysEdit->setValue(settings.value(OrderSettings::oldMarketOrderDaysKey, OrderSettings::oldMarketOrderDaysDefault).toUInt());
        mOldMarketOrdersDaysEdit->setEnabled(mDeleteOldMarketOrdersBtn->isChecked());

        mLimitSellToStationBtn = new QCheckBox{tr("Limit sell order price checking to station"), this};
        mainGroupLayout->addRow(mLimitSellToStationBtn);
        mLimitSellToStationBtn->setChecked(settings.value(OrderSettings::limitSellToStationKey, OrderSettings::limitSellToStationDefault).toBool());

        mVolumeWarningEdit = new QSpinBox{this};
        mainGroupLayout->addRow(tr("Order volume warning (0 for none):"), mVolumeWarningEdit);
        mVolumeWarningEdit->setRange(0, 100);
        mVolumeWarningEdit->setSuffix(locale().percent());
        mVolumeWarningEdit->setValue(settings.value(OrderSettings::volumeWarningKey, OrderSettings::volumeWarningDefault).toInt());

        auto customStationGroup = new QGroupBox{this};
        mainLayout->addWidget(customStationGroup);

        auto customStationGroupLayout = new QFormLayout{customStationGroup};

        mDefaultCustomStation = settings.value(OrderSettings::defaultCustomStationKey);

        mDefaultCustomStationBtn = new QPushButton{(mDefaultCustomStation.isNull()) ? (tr("none")) : (mDataProvider.getLocationName(mDefaultCustomStation.toULongLong())), this};
        customStationGroupLayout->addRow(tr("Default custom station:"), mDefaultCustomStationBtn);
        connect(mDefaultCustomStationBtn, &QPushButton::clicked, this, &OrderPreferencesWidget::chooseDefaultCustomStation);

        const auto customStationHelp = new QLabel{
            tr("If you change the default custom station, your orders will still retain the one previously set. You can change it by right-clicking on the order(s)."), this
        };
        customStationGroupLayout->addRow(customStationHelp);
        customStationHelp->setWordWrap(true);

        mImportFromCitadels = new QCheckBox{tr("Try to import orders from citadels"), this};
        customStationGroupLayout->addRow(mImportFromCitadels);
        mImportFromCitadels->setChecked(settings.value(OrderSettings::importFromCitadelsKey, OrderSettings::importFromCitadelsDefault).toBool());

        mainLayout->addStretch();
    }

    void OrderPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(OrderSettings::marketOrderMaxAgeKey, mMarketOrderMaxAgeEdit->value());
        settings.setValue(OrderSettings::deleteOldMarketOrdersKey, mDeleteOldMarketOrdersBtn->isChecked());
        settings.setValue(OrderSettings::oldMarketOrderDaysKey, mOldMarketOrdersDaysEdit->value());
        settings.setValue(OrderSettings::limitSellToStationKey, mLimitSellToStationBtn->isChecked());
        settings.setValue(OrderSettings::volumeWarningKey, mVolumeWarningEdit->value());
        settings.setValue(OrderSettings::defaultCustomStationKey, mDefaultCustomStation);
        settings.setValue(OrderSettings::importFromCitadelsKey, mImportFromCitadels->isChecked());
    }

    void OrderPreferencesWidget::chooseDefaultCustomStation()
    {
        StationSelectDialog dlg{mDataProvider, true, this};
        if (dlg.exec() == QDialog::Accepted)
        {
            const auto id = dlg.getStationId();

            mDefaultCustomStation = id;
            mDefaultCustomStationBtn->setText((id == 0) ? (tr("none")) : (mDataProvider.getLocationName(id)));
        }
    }
}
