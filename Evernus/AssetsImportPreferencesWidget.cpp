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

#include "AssetsImportPreferencesWidget.h"

namespace Evernus
{
    AssetsImportPreferencesWidget::AssetsImportPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto importBox = new QGroupBox{tr("Assets import"), this};
        mainLayout->addWidget(importBox);

        auto importBoxLayout = new QVBoxLayout{importBox};

        mImportAssetsBox = new QCheckBox{tr("Import assets"), this};
        importBoxLayout->addWidget(mImportAssetsBox);
        mImportAssetsBox->setChecked(settings.value(ImportSettings::importAssetsKey, ImportSettings::importAssetsDefault).toBool());

        mAutoUpdateValueBox = new QCheckBox{tr("Store total asset value on import/price import"), this};
        importBoxLayout->addWidget(mAutoUpdateValueBox);
        mAutoUpdateValueBox->setChecked(
            settings.value(ImportSettings::autoUpdateAssetValueKey, ImportSettings::autoUpdateAssetValueDefault).toBool());

        mUpdateOnlyFullValueBox = new QCheckBox{tr("Store total asset value only when all prices are available"), this};
        importBoxLayout->addWidget(mUpdateOnlyFullValueBox);
        mUpdateOnlyFullValueBox->setChecked(
            settings.value(ImportSettings::updateOnlyFullAssetValueKey, ImportSettings::updateOnlyFullAssetValueDefault).toBool());

        mainLayout->addStretch();
    }

    void AssetsImportPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::importAssetsKey, mImportAssetsBox->isChecked());
        settings.setValue(ImportSettings::autoUpdateAssetValueKey, mAutoUpdateValueBox->isChecked());
        settings.setValue(ImportSettings::updateOnlyFullAssetValueKey, mUpdateOnlyFullValueBox->isChecked());
    }
}
