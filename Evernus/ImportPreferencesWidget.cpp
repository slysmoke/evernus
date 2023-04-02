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

#include <QMessageBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>

#include "ImportPreferencesWidget.h"

namespace Evernus
{
    ImportPreferencesWidget::ImportPreferencesWidget(QWidget *parent)
        : QWidget(parent)
        , mCrypt(ImportSettings::smtpCryptKey)
    {
        QSettings settings;

        auto mainLayout = new QVBoxLayout{this};

        auto generalBox = new QGroupBox{this};
        mainLayout->addWidget(generalBox);

        auto generalBoxLayout = new QFormLayout{generalBox};

        mIgnoreCachedBtn = new QCheckBox{tr("Ignore up-to-date data"), this};
        generalBoxLayout->addRow(mIgnoreCachedBtn);
        mIgnoreCachedBtn->setChecked(settings.value(ImportSettings::ignoreCachedImportKey, ImportSettings::ignoreCachedImportDefault).toBool());

        mAllCharactersBtn = new QCheckBox{tr("Import data for all characters with one click"), this};
        generalBoxLayout->addRow(mAllCharactersBtn);
        mAllCharactersBtn->setChecked(settings.value(ImportSettings::importAllCharactersKey, ImportSettings::importAllCharactersDefault).toBool());

        mClearExistingCitadelsBtn = new QCheckBox{tr("Clear existing citadels on import"), this};
        generalBoxLayout->addRow(mClearExistingCitadelsBtn);
        mClearExistingCitadelsBtn->setChecked(settings.value(ImportSettings::clearExistingCitadelsKey, ImportSettings::clearExistingCitadelsDefault).toBool());

        mCsvSeparatorEdit = new QLineEdit{settings.value(ImportSettings::csvSeparatorKey, ImportSettings::csvSeparatorDefault).toString(), this};
        generalBoxLayout->addRow(tr("CSV separator:"), mCsvSeparatorEdit);
        mCsvSeparatorEdit->setMaxLength(1);

        auto timersBox = new QGroupBox{tr("Data age"), this};
        mainLayout->addWidget(timersBox);

        auto timersBoxLayout = new QFormLayout{timersBox};

        mCharacterTimerEdit = createTimerSpin(settings.value(ImportSettings::maxCharacterAgeKey, ImportSettings::importTimerDefault).toInt());
        timersBoxLayout->addRow(tr("Max. character update age:"), mCharacterTimerEdit);

        mAssetListTimerEdit = createTimerSpin(settings.value(ImportSettings::maxAssetListAgeKey, ImportSettings::importTimerDefault).toInt());
        timersBoxLayout->addRow(tr("Max. asset list update age:"), mAssetListTimerEdit);

        mWalletTimerEdit = createTimerSpin(settings.value(ImportSettings::maxWalletAgeKey, ImportSettings::importTimerDefault).toInt());
        timersBoxLayout->addRow(tr("Max. wallet update age:"), mWalletTimerEdit);

        mMarketOrdersTimerEdit = createTimerSpin(settings.value(ImportSettings::maxMarketOrdersAgeKey, ImportSettings::importTimerDefault).toInt());
        timersBoxLayout->addRow(tr("Max. market orders update age:"), mMarketOrdersTimerEdit);

        mContractsTimerEdit = createTimerSpin(settings.value(ImportSettings::maxContractsAgeKey, ImportSettings::importTimerDefault).toInt());
        timersBoxLayout->addRow(tr("Max. contracts update age:"), mContractsTimerEdit);

        mCitadelAccessTimeEdit = new QSpinBox{this};
        mCitadelAccessTimeEdit->setRange(1, 365);
        mCitadelAccessTimeEdit->setSuffix(tr(" days"));
        mCitadelAccessTimeEdit->setValue(settings.value(ImportSettings::maxCitadelAccessAgeKey, ImportSettings::maxCitadelAccessAgeDefault).toInt());
        timersBoxLayout->addRow(tr("Max. citadel access cache age:"), mCitadelAccessTimeEdit);

        auto autoImportBox = new QGroupBox{tr("Auto-import"), this};
        mainLayout->addWidget(autoImportBox);

        auto autoImportBoxLayout = new QFormLayout{autoImportBox};

        mAutoImportBtn = new QCheckBox{tr("Enabled"), this};
        autoImportBoxLayout->addRow(mAutoImportBtn);
        mAutoImportBtn->setChecked(settings.value(ImportSettings::autoImportEnabledKey, ImportSettings::autoImportEnabledDefault).toBool());

        mAutoImportTimeEdit = new QSpinBox{this};
        autoImportBoxLayout->addRow(tr("Auto-import time:"), mAutoImportTimeEdit);
        mAutoImportTimeEdit->setMinimum(5);
        mAutoImportTimeEdit->setMaximum(24 * 60);
        mAutoImportTimeEdit->setSuffix(tr("min"));
        mAutoImportTimeEdit->setValue(settings.value(ImportSettings::autoImportTimeKey, ImportSettings::autoImportTimerDefault).toInt());

        mEmailNotificationBtn = new QCheckBox{tr("Enable email notifications"), this};
        autoImportBoxLayout->addRow(mEmailNotificationBtn);
        mEmailNotificationBtn->setChecked(
            settings.value(ImportSettings::emailNotificationsEnabledKey, ImportSettings::emailNotificationsEnabledDefault).toBool());

        mEmailNotificationAddressEdit = new QLineEdit{settings.value(ImportSettings::emailNotificationAddressKey).toString(), this};
        autoImportBoxLayout->addRow(tr("Destination address:"), mEmailNotificationAddressEdit);

        const auto smtpConnectionSecurity = static_cast<ImportSettings::SmtpConnectionSecurity>(
            settings.value(ImportSettings::smtpConnectionSecurityKey).toInt());

        mSmtpConnectionSecurityEdit = new QComboBox{this};
        autoImportBoxLayout->addRow(tr("SMTP security:"), mSmtpConnectionSecurityEdit);
        addSmtpConnectionSecurityItem(tr("None"), ImportSettings::SmtpConnectionSecurity::None, smtpConnectionSecurity);
        addSmtpConnectionSecurityItem(tr("STARTTLS"), ImportSettings::SmtpConnectionSecurity::STARTTLS, smtpConnectionSecurity);
        addSmtpConnectionSecurityItem(tr("SSL/TLS"), ImportSettings::SmtpConnectionSecurity::TLS, smtpConnectionSecurity);

        mSmtpHostEdit = new QLineEdit{settings.value(ImportSettings::smtpHostKey, ImportSettings::smtpHostDefault).toString(), this};
        autoImportBoxLayout->addRow(tr("SMTP host:"), mSmtpHostEdit);

        mSmtpPortEdit = new QSpinBox{this};
        autoImportBoxLayout->addRow(tr("SMTP port:"), mSmtpPortEdit);
        mSmtpPortEdit->setMinimum(1);
        mSmtpPortEdit->setMaximum(std::numeric_limits<quint16>::max());
        mSmtpPortEdit->setValue(settings.value(ImportSettings::smtpPortKey, ImportSettings::smtpPortDefault).toInt());

        mSmtpUserEdit = new QLineEdit{settings.value(ImportSettings::smtpUserKey).toString(), this};
        autoImportBoxLayout->addRow(tr("SMTP user:"), mSmtpUserEdit);

        mSmtpPasswordEdit = new QLineEdit{mCrypt.decryptToString(settings.value(ImportSettings::smtpPasswordKey).toString()), this};
        autoImportBoxLayout->addRow(tr("SMTP password:"), mSmtpPasswordEdit);
        mSmtpPasswordEdit->setEchoMode(QLineEdit::Password);

        auto warningLabel = new QLabel{tr("Warning: password store uses weak encryption - do not use sensitive passwords."), this};
        autoImportBoxLayout->addRow(warningLabel);
        warningLabel->setWordWrap(true);

        mainLayout->addStretch();
    }

    void ImportPreferencesWidget::applySettings()
    {
        const auto autoImport = mAutoImportBtn->isChecked();
        auto emailNotifications = mEmailNotificationBtn->isChecked();
        const auto emailNotificationsAddress = mEmailNotificationAddressEdit->text();
        const auto smtpHost = mSmtpHostEdit->text();

        if (autoImport && emailNotifications && (emailNotificationsAddress.isEmpty() || smtpHost.isEmpty()))
        {
            QMessageBox::warning(this, tr("Email notifications"), tr("Email notifications will be disabled since destination address or SMTP host is empty."));
            emailNotifications = false;
        }

        auto csvSeparator = mCsvSeparatorEdit->text();
        if (csvSeparator.length() != 1)
            csvSeparator = ImportSettings::csvSeparatorDefault;

        QSettings settings;
        settings.setValue(ImportSettings::ignoreCachedImportKey, mIgnoreCachedBtn->isChecked());
        settings.setValue(ImportSettings::importAllCharactersKey, mAllCharactersBtn->isChecked());
        settings.setValue(ImportSettings::clearExistingCitadelsKey, mClearExistingCitadelsBtn->isChecked());
        settings.setValue(ImportSettings::csvSeparatorKey, csvSeparator);
        settings.setValue(ImportSettings::maxCharacterAgeKey, mCharacterTimerEdit->value());
        settings.setValue(ImportSettings::maxAssetListAgeKey, mAssetListTimerEdit->value());
        settings.setValue(ImportSettings::maxWalletAgeKey, mWalletTimerEdit->value());
        settings.setValue(ImportSettings::maxMarketOrdersAgeKey, mMarketOrdersTimerEdit->value());
        settings.setValue(ImportSettings::maxContractsAgeKey, mContractsTimerEdit->value());
        settings.setValue(ImportSettings::maxCitadelAccessAgeKey, mCitadelAccessTimeEdit->value());
        settings.setValue(ImportSettings::autoImportEnabledKey, autoImport);
        settings.setValue(ImportSettings::autoImportTimeKey, mAutoImportTimeEdit->value());
        settings.setValue(ImportSettings::emailNotificationsEnabledKey, emailNotifications);
        settings.setValue(ImportSettings::emailNotificationAddressKey, emailNotificationsAddress);
        settings.setValue(ImportSettings::smtpConnectionSecurityKey, mSmtpConnectionSecurityEdit->currentData().toInt());
        settings.setValue(ImportSettings::smtpHostKey, smtpHost);
        settings.setValue(ImportSettings::smtpPortKey, mSmtpPortEdit->value());
        settings.setValue(ImportSettings::smtpUserKey, mSmtpUserEdit->text());
        settings.setValue(ImportSettings::smtpPasswordKey, mCrypt.encryptToString(mSmtpPasswordEdit->text()));
    }

    QSpinBox *ImportPreferencesWidget::createTimerSpin(int value)
    {
        auto spin = new QSpinBox{this};
        spin->setRange(1, 60 * 24);
        spin->setSuffix(tr("min"));
        spin->setValue(value);

        return spin;
    }

    void ImportPreferencesWidget::addSmtpConnectionSecurityItem(const QString &label,
                                                                ImportSettings::SmtpConnectionSecurity value,
                                                                ImportSettings::SmtpConnectionSecurity curValue)
    {
        mSmtpConnectionSecurityEdit->addItem(label, static_cast<int>(value));
        if (curValue == value)
            mSmtpConnectionSecurityEdit->setCurrentIndex(mSmtpConnectionSecurityEdit->count() - 1);
    }
}
