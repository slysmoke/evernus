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
#include <QLabel>

#include "ImportSettings.h"

#include "ContractImportPreferencesWidget.h"

namespace Evernus
{
    ContractImportPreferencesWidget::ContractImportPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto importBox = new QGroupBox{tr("Contract import"), this};
        mainLayout->addWidget(importBox);

        auto importBoxLayout = new QVBoxLayout{importBox};

        mImportContractBox = new QCheckBox{tr("Import contracts"), this};
        importBoxLayout->addWidget(mImportContractBox);
        mImportContractBox->setChecked(settings.value(ImportSettings::importContractsKey, ImportSettings::importContractsDefault).toBool());

        mainLayout->addStretch();
    }

    void ContractImportPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::importContractsKey, mImportContractBox->isChecked());
    }
}
