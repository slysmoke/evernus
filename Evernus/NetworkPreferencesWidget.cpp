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
#include <limits>

#include <QNetworkProxy>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <QLineEdit>
#include <QSettings>
#include <QSpinBox>
#include <QLabel>

#include "NetworkSettings.h"

#include "NetworkPreferencesWidget.h"

namespace Evernus
{
    NetworkPreferencesWidget::NetworkPreferencesWidget(QWidget *parent)
        : QWidget(parent)
        , mCrypt(NetworkSettings::cryptKey)
    {
        QSettings settings;

        const auto useProxy = settings.value(NetworkSettings::useProxyKey, NetworkSettings::useProxyDefault).toBool();
        const auto proxyType = settings.value(NetworkSettings::proxyTypeKey).toInt();
        const auto proxyHost = settings.value(NetworkSettings::proxyHostKey).toString();
        const quint16 proxyPort = settings.value(NetworkSettings::proxyPortKey).toUInt();
        const auto proxyUser = settings.value(NetworkSettings::proxyUserKey).toString();
        const auto proxyPassword = mCrypt.decryptToString(settings.value(NetworkSettings::proxyPasswordKey).toString());

        auto mainLayout = new QVBoxLayout{this};

        auto proxyGroup = new QGroupBox{tr("Proxy"), this};
        mainLayout->addWidget(proxyGroup);

        auto proxyLayout = new QVBoxLayout{proxyGroup};

        mNoProxyBtn = new QRadioButton{tr("No proxy"), this};
        proxyLayout->addWidget(mNoProxyBtn);

        auto customProxyBtn = new QRadioButton{tr("Custom proxy"), this};
        proxyLayout->addWidget(customProxyBtn);

        auto proxySettingsContainerWidget = new QWidget{this};
        proxyLayout->addWidget(proxySettingsContainerWidget);
        connect(customProxyBtn, &QRadioButton::toggled, proxySettingsContainerWidget, &QWidget::setEnabled);

        auto proxySettingLayout = new QFormLayout{proxySettingsContainerWidget};

        mProxyTypeCombo = new QComboBox{this};
        proxySettingLayout->addRow(tr("Type:"), mProxyTypeCombo);
        mProxyTypeCombo->addItem(tr("SOCKS5"), QNetworkProxy::Socks5Proxy);
        mProxyTypeCombo->addItem(tr("HTTP"), QNetworkProxy::HttpProxy);

        mProxyHostEdit = new QLineEdit{proxyHost, this};
        proxySettingLayout->addRow(tr("Host:"), mProxyHostEdit);

        mProxyPortEdit = new QSpinBox{this};
        proxySettingLayout->addRow(tr("Port:"), mProxyPortEdit);
        mProxyPortEdit->setMinimum(1);
        mProxyPortEdit->setMaximum(std::numeric_limits<quint16>::max());
        mProxyPortEdit->setValue(proxyPort);

        mProxyUserEdit = new QLineEdit{proxyUser, this};
        proxySettingLayout->addRow(tr("User:"), mProxyUserEdit);

        mProxyPasswordEdit = new QLineEdit{proxyPassword, this};
        proxySettingLayout->addRow(tr("Password:"), mProxyPasswordEdit);
        mProxyPasswordEdit->setEchoMode(QLineEdit::Password);

        auto warningLabel = new QLabel{tr("Warning: password store uses weak encryption - do not use sensitive passwords."), this};
        proxyLayout->addWidget(warningLabel);
        warningLabel->setWordWrap(true);

        auto miscGroup = new QGroupBox{this};
        mainLayout->addWidget(miscGroup);

        auto miscGroupLayout = new QFormLayout{miscGroup};

        mMaxReplyTimeEdit = new QSpinBox{this};
        miscGroupLayout->addRow(tr("Max. reply time:"), mMaxReplyTimeEdit);
        mMaxReplyTimeEdit->setMaximum(10000);
        mMaxReplyTimeEdit->setSuffix("s");
        mMaxReplyTimeEdit->setValue(
            settings.value(NetworkSettings::maxReplyTimeKey, NetworkSettings::maxReplyTimeDefault).toUInt());

        mMaxRetriesEdit = new QSpinBox{this};
        miscGroupLayout->addRow(tr("Max. retries:"), mMaxRetriesEdit);
        mMaxRetriesEdit->setValue(
            settings.value(NetworkSettings::maxRetriesKey, NetworkSettings::maxRetriesDefault).toUInt());

        mIgnoreSslErrors = new QCheckBox{tr("Ignore certificate errors"), this};
        miscGroupLayout->addRow(mIgnoreSslErrors);
        mIgnoreSslErrors->setChecked(
            settings.value(NetworkSettings::ignoreSslErrorsKey, NetworkSettings::ignoreSslErrorsDefault).toBool());

        mLogESIReplies = new QCheckBox{tr("Log ESI replies"), this};
        miscGroupLayout->addRow(mLogESIReplies);
        mLogESIReplies->setChecked(
            settings.value(NetworkSettings::logESIRepliesKey, NetworkSettings::logESIRepliesDefault).toBool());

        mUseHTTP2 = new QCheckBox{tr("Use HTTP/2"), this};
        miscGroupLayout->addRow(mUseHTTP2);
        mUseHTTP2->setChecked(
            settings.value(NetworkSettings::useHTTP2Key, NetworkSettings::useHTTP2Default).toBool());

        const auto clearTokensBtn = new QPushButton{tr("Clear saved refresh tokens"), this};
        miscGroupLayout->addRow(clearTokensBtn);
        connect(clearTokensBtn, &QPushButton::clicked, this, &NetworkPreferencesWidget::clearRefreshTokens);

        mainLayout->addStretch();

        if (useProxy)
        {
            customProxyBtn->setChecked(true);
        }
        else
        {
            mNoProxyBtn->setChecked(true);
            proxySettingsContainerWidget->setDisabled(true);
        }

        if (proxyType == QNetworkProxy::HttpProxy)
            mProxyTypeCombo->setCurrentIndex(1);
    }

    void NetworkPreferencesWidget::applySettings()
    {
        QSettings settings;

        const auto useProxy = !mNoProxyBtn->isChecked();
        const auto proxyType = mProxyTypeCombo->itemData(mProxyTypeCombo->currentIndex()).toInt();
        const auto proxyHost = mProxyHostEdit->text();
        const quint16 proxyPort = mProxyPortEdit->value();
        const auto proxyUser = mProxyUserEdit->text();
        const auto proxyPassword = mProxyPasswordEdit->text();

        settings.setValue(NetworkSettings::useProxyKey, useProxy);
        settings.setValue(NetworkSettings::proxyTypeKey, proxyType);
        settings.setValue(NetworkSettings::proxyHostKey, proxyHost);
        settings.setValue(NetworkSettings::proxyPortKey, proxyPort);
        settings.setValue(NetworkSettings::proxyUserKey, proxyUser);
        settings.setValue(NetworkSettings::proxyPasswordKey, mCrypt.encryptToString(proxyPassword));

        settings.setValue(NetworkSettings::maxReplyTimeKey, mMaxReplyTimeEdit->value());
        settings.setValue(NetworkSettings::maxRetriesKey, mMaxRetriesEdit->value());
        settings.setValue(NetworkSettings::ignoreSslErrorsKey, mIgnoreSslErrors->isChecked());
        settings.setValue(NetworkSettings::logESIRepliesKey, mLogESIReplies->isChecked());
        settings.setValue(NetworkSettings::useHTTP2Key, mUseHTTP2->isChecked());
    }
}
