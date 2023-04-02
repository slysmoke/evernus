/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Http Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Http Public License for more details.
 *
 *  You should have received a copy of the GNU Http Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <limits>

#include <QFormLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>

#include "HttpSettings.h"

#include "HttpPreferencesWidget.h"

namespace Evernus
{
    HttpPreferencesWidget::HttpPreferencesWidget(QWidget *parent)
        : QWidget(parent)
        , mCrypt(HttpSettings::cryptKey)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto mainGroup = new QGroupBox{this};
        mainLayout->addWidget(mainGroup);

        auto mainGroupLayout = new QFormLayout{mainGroup};

        mEnabledBtn = new QCheckBox{tr("Enabled"), this};
        mainGroupLayout->addRow(mEnabledBtn);
        mEnabledBtn->setChecked(settings.value(HttpSettings::enabledKey, HttpSettings::enabledDefault).toBool());

        mPortEdit = new QSpinBox{this};
        mainGroupLayout->addRow(tr("Port:"), mPortEdit);
        mPortEdit->setMinimum(1);
        mPortEdit->setMaximum(std::numeric_limits<quint16>::max());
        mPortEdit->setValue(settings.value(HttpSettings::portKey, HttpSettings::portDefault).toInt());

        mHttpUserEdit = new QLineEdit{settings.value(HttpSettings::userKey).toString(), this};
        mainGroupLayout->addRow(tr("HTTP user:"), mHttpUserEdit);

        mHttpPasswordEdit = new QLineEdit{mCrypt.decryptToString(settings.value(HttpSettings::passwordKey).toString()), this};
        mainGroupLayout->addRow(tr("HTTP password:"), mHttpPasswordEdit);
        mHttpPasswordEdit->setEchoMode(QLineEdit::Password);

        auto warningLabel = new QLabel{tr("Warning: password store uses weak encryption - do not use sensitive passwords."), this};
        mainGroupLayout->addRow(warningLabel);
        warningLabel->setWordWrap(true);

        mainLayout->addStretch();
    }

    void HttpPreferencesWidget::applySettings()
    {
        QSettings settings;
        settings.setValue(HttpSettings::enabledKey, mEnabledBtn->isChecked());
        settings.setValue(HttpSettings::portKey, mPortEdit->value());
        settings.setValue(HttpSettings::userKey, mHttpUserEdit->text());
        settings.setValue(HttpSettings::passwordKey, mCrypt.encryptToString(mHttpPasswordEdit->text()));
    }
}
