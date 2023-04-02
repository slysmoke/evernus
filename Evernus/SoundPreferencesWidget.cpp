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
#include <QSettings>
#include <QGroupBox>
#include <QCheckBox>

#include "SoundSettings.h"

#include "SoundPreferencesWidget.h"

namespace Evernus
{
    SoundPreferencesWidget::SoundPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto mainGroup = new QGroupBox{this};
        mainLayout->addWidget(mainGroup);

        auto mainGroupLayout = new QVBoxLayout{mainGroup};

        QSettings settings;

        mFPCSoundBtn = new QCheckBox{tr("Fast Price Copy sound"), this};
        mainGroupLayout->addWidget(mFPCSoundBtn);
        mFPCSoundBtn->setChecked(settings.value(SoundSettings::fpcSoundKey, SoundSettings::fpcSoundDefault).toBool());

        mainLayout->addStretch();
    }

    void SoundPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(SoundSettings::fpcSoundKey, mFPCSoundBtn->isChecked());
    }
}
