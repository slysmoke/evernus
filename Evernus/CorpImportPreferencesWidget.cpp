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
#include <QMessageBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QComboBox>
#include <QSettings>

#include "ImportSettings.h"

#include "CorpImportPreferencesWidget.h"

namespace Evernus
{
    CorpImportPreferencesWidget::CorpImportPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto importBox = new QGroupBox{tr("Corporation data import"), this};
        mainLayout->addWidget(importBox);

        auto importBoxLayout = new QVBoxLayout{importBox};

        mUpdateCorpDataBtn = new QCheckBox{tr("Import corporation data along with character"), this};
        importBoxLayout->addWidget(mUpdateCorpDataBtn);
        mUpdateCorpDataBtn->setChecked(settings.value(ImportSettings::updateCorpDataKey).toBool());

        mMakeCorpSnapshotsBtn = new QCheckBox{tr("Make value snapshots from corporation data"), this};
        importBoxLayout->addWidget(mMakeCorpSnapshotsBtn);
        mMakeCorpSnapshotsBtn->setChecked(
            settings.value(ImportSettings::makeCorpSnapshotsKey, ImportSettings::makeCorpSnapshotsDefault).toBool());

        mShowCorpOrdersWithCharacterBtn = new QCheckBox{tr("Show corporation orders with character's"), this};
        importBoxLayout->addWidget(mShowCorpOrdersWithCharacterBtn);
        mShowCorpOrdersWithCharacterBtn->setChecked(
            settings.value(ImportSettings::corpOrdersWithCharacterKey, ImportSettings::corpOrdersWithCharacterDefault).toBool());

        auto importFormLayout = new QFormLayout{};
        importBoxLayout->addLayout(importFormLayout);

        mWalletDivisionCombo = new QComboBox{this};
        importFormLayout->addRow(tr("Wallet division:"), mWalletDivisionCombo);
        mWalletDivisionCombo->addItem(tr("Master Wallet"), ImportSettings::corpWalletDivisionDefault);
        mWalletDivisionCombo->addItem(tr("2nd Wallet Division"), ImportSettings::corpWalletDivisionDefault + 1);
        mWalletDivisionCombo->addItem(tr("3rd Wallet Division"), ImportSettings::corpWalletDivisionDefault + 2);
        mWalletDivisionCombo->addItem(tr("4th Wallet Division"), ImportSettings::corpWalletDivisionDefault + 3);
        mWalletDivisionCombo->addItem(tr("5th Wallet Division"), ImportSettings::corpWalletDivisionDefault + 4);
        mWalletDivisionCombo->addItem(tr("6th Wallet Division"), ImportSettings::corpWalletDivisionDefault + 5);
        mWalletDivisionCombo->addItem(tr("7th Wallet Division"), ImportSettings::corpWalletDivisionDefault + 6);
        mWalletDivisionCombo->setCurrentIndex(mWalletDivisionCombo->findData(
            settings.value(ImportSettings::corpWalletDivisionKey, ImportSettings::corpWalletDivisionDefault).toInt()));
        connect(mWalletDivisionCombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, [=] {
            const auto ret = QMessageBox::question(this,
                                                   tr("Wallet changed"),
                                                   tr("Do you wish to remove previous corporation wallet data?"));
            if (ret == QMessageBox::Yes)
                emit clearCorpWalletData();
        });

        mainLayout->addStretch();
    }

    void CorpImportPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::updateCorpDataKey, mUpdateCorpDataBtn->isChecked());
        settings.setValue(ImportSettings::makeCorpSnapshotsKey, mMakeCorpSnapshotsBtn->isChecked());
        settings.setValue(ImportSettings::corpOrdersWithCharacterKey, mShowCorpOrdersWithCharacterBtn->isChecked());
        settings.setValue(ImportSettings::corpWalletDivisionKey, mWalletDivisionCombo->currentData());
    }
}
