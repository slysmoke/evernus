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
#pragma once

#include <QWidget>

#include "ImportSettings.h"
#include "SimpleCrypt.h"

class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;

namespace Evernus
{
    class ImportPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit ImportPreferencesWidget(QWidget *parent = nullptr);
        virtual ~ImportPreferencesWidget() = default;

    public slots:
        void applySettings();

    private:
        SimpleCrypt mCrypt;

        QCheckBox *mIgnoreCachedBtn = nullptr;
        QCheckBox *mAllCharactersBtn = nullptr;
        QCheckBox *mClearExistingCitadelsBtn = nullptr;

        QSpinBox *mCharacterTimerEdit = nullptr;
        QSpinBox *mAssetListTimerEdit = nullptr;
        QSpinBox *mWalletTimerEdit = nullptr;
        QSpinBox *mMarketOrdersTimerEdit = nullptr;
        QSpinBox *mContractsTimerEdit = nullptr;
        QSpinBox *mCitadelAccessTimeEdit = nullptr;

        QCheckBox *mAutoImportBtn = nullptr;
        QSpinBox *mAutoImportTimeEdit = nullptr;
        QLineEdit *mCsvSeparatorEdit = nullptr;
        QCheckBox *mEmailNotificationBtn = nullptr;
        QLineEdit *mEmailNotificationAddressEdit = nullptr;
        QComboBox *mSmtpConnectionSecurityEdit = nullptr;
        QLineEdit *mSmtpHostEdit = nullptr;
        QSpinBox *mSmtpPortEdit = nullptr;
        QLineEdit *mSmtpUserEdit = nullptr;
        QLineEdit *mSmtpPasswordEdit = nullptr;

        QSpinBox *createTimerSpin(int value);

        void addSmtpConnectionSecurityItem(const QString &label,
                                           ImportSettings::SmtpConnectionSecurity value,
                                           ImportSettings::SmtpConnectionSecurity curValue);
    };
}
