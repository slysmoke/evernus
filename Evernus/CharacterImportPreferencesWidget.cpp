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

#include "CharacterImportPreferencesWidget.h"

namespace Evernus
{
    CharacterImportPreferencesWidget::CharacterImportPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto importBox = new QGroupBox{tr("Character import"), this};
        mainLayout->addWidget(importBox);

        auto importBoxLayout = new QVBoxLayout{importBox};

        mImportSkillsBox = new QCheckBox{tr("Import skills"), this};
        importBoxLayout->addWidget(mImportSkillsBox);
        mImportSkillsBox->setChecked(settings.value(ImportSettings::importSkillsKey, ImportSettings::importSkillsDefault).toBool());

        mImportPortraitBox = new QCheckBox{tr("Import portrait"), this};
        importBoxLayout->addWidget(mImportPortraitBox);
        mImportPortraitBox->setChecked(settings.value(ImportSettings::importPortraitKey, ImportSettings::importPortraitDefault).toBool());

        mainLayout->addStretch();
    }

    void CharacterImportPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(ImportSettings::importSkillsKey, mImportSkillsBox->isChecked());
        settings.setValue(ImportSettings::importPortraitKey, mImportPortraitBox->isChecked());
    }
}
