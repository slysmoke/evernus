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
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>

#include "ImportSettings.h"

#include "IndustryImportPreferencesWidget.h"

namespace Evernus
{
    IndustryImportPreferencesWidget::IndustryImportPreferencesWidget(QWidget *parent)
        : QWidget{parent}
    {
        const auto mainLayout = new QVBoxLayout{this};

        const auto importGroup = new QGroupBox{this};
        mainLayout->addWidget(importGroup);

        const auto importGroupLayout = new QVBoxLayout{importGroup};

        QSettings settings;

        mImportMiningLedger = new QCheckBox{tr("Import mining ledger"), this};
        importGroupLayout->addWidget(mImportMiningLedger);
        mImportMiningLedger->setChecked(
            settings.value(ImportSettings::importMiningLedgerKey, ImportSettings::importMiningLedgerDefault).toBool()
        );

        mainLayout->addStretch();
    }

    void IndustryImportPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::importMiningLedgerKey, mImportMiningLedger->isChecked());
    }
}
