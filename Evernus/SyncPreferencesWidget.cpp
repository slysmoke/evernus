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
#include <QPushButton>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QLabel>

#include "SyncSettings.h"

#include "SyncPreferencesWidget.h"

namespace Evernus
{
    SyncPreferencesWidget::SyncPreferencesWidget(QWidget *parent)
        : QWidget(parent)
    {
        auto mainLayout = new QVBoxLayout{this};

#ifdef EVERNUS_DROPBOX_ENABLED
        QSettings settings;

        auto dropboxGroup = new QGroupBox{this};
        mainLayout->addWidget(dropboxGroup);

        auto dropboxGroupLayout = new QVBoxLayout{dropboxGroup};

        mEnabledOnStartupBtn = new QCheckBox{tr("Download on startup"), this};
        dropboxGroupLayout->addWidget(mEnabledOnStartupBtn);
        mEnabledOnStartupBtn->setChecked(settings.value(SyncSettings::enabledOnStartupKey, SyncSettings::enabledOnStartupDefault).toBool());

        mEnabledOnShutdownBtn = new QCheckBox{tr("Upload on shutdown"), this};
        dropboxGroupLayout->addWidget(mEnabledOnShutdownBtn);
        mEnabledOnShutdownBtn->setChecked(settings.value(SyncSettings::enabledOnShutdownKey, SyncSettings::enabledOnShutdownDefault).toBool());
        mEnabledOnShutdownBtn->setEnabled(mEnabledOnStartupBtn->isChecked());
        connect(mEnabledOnStartupBtn, &QCheckBox::toggled, mEnabledOnShutdownBtn, &QCheckBox::setEnabled);

        const auto clearBtn = new QPushButton{tr("Clear saved tokens"), this};
        dropboxGroupLayout->addWidget(clearBtn);
        connect(clearBtn, &QPushButton::clicked, this, [=] {
            QSettings settings;
            settings.remove(SyncSettings::dbTokenKey);
        });
#else
        auto info = new QLabel{tr(
            "Evernus was compiled without Dropbox app key and secret values. In order to enable Dropbox support, register "
            "a new app key with Dropbox and recompile Evernus with EVERNUS_DROPBOX_APP_KEY and EVERNUS_DROPBOX_APP_SECRET "
            "defined."
        ), this};
        info->setWordWrap(true);

        mainLayout->addWidget(info);
#endif

        mainLayout->addStretch();
    }

    void SyncPreferencesWidget::applySettings()
    {
#ifdef EVERNUS_DROPBOX_ENABLED
        QSettings settings;
        settings.setValue(SyncSettings::enabledOnStartupKey, mEnabledOnStartupBtn->isChecked());
        settings.setValue(SyncSettings::enabledOnShutdownKey, mEnabledOnShutdownBtn->isChecked());
#endif
    }
}
