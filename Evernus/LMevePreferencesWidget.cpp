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
#include <QLineEdit>
#include <QLabel>

#include "LMeveSettings.h"

#include "LMevePreferencesWidget.h"

namespace Evernus
{
    LMevePreferencesWidget::LMevePreferencesWidget(QWidget *parent)
        : QWidget(parent)
        , mCrypt(LMeveSettings::lmeveCryptKey)
    {
        auto mainLayout = new QVBoxLayout{this};

        auto mainGroup = new QGroupBox{this};
        mainLayout->addWidget(mainGroup);

        auto mainGroupLayout = new QFormLayout{mainGroup};

        auto infoLabel
            = new QLabel{tr("To start working with LMeve, visit its <a href='https://github.com/roxlukas/lmeve'>homepage</a>."), this};
        mainGroupLayout->addRow(infoLabel);
        infoLabel->setWordWrap(true);
        infoLabel->setOpenExternalLinks(true);

        QSettings settings;

        mUrlEdit = new QLineEdit{settings.value(LMeveSettings::urlKey).toString(), this};
        mainGroupLayout->addRow(tr("Url:"), mUrlEdit);

        mKeyEdit = new QLineEdit{mCrypt.decryptToString(settings.value(LMeveSettings::keyKey).toString()), this};
        mainGroupLayout->addRow(tr("Key:"), mKeyEdit);

        mainLayout->addStretch();
    }

    void LMevePreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(LMeveSettings::urlKey, mUrlEdit->text());
        settings.setValue(LMeveSettings::keyKey, mCrypt.encryptToString(mKeyEdit->text()));
    }
}
