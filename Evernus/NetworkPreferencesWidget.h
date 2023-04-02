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

#include "SimpleCrypt.h"

class QRadioButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QSpinBox;

namespace Evernus
{
    class NetworkPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit NetworkPreferencesWidget(QWidget *parent = nullptr);
        virtual ~NetworkPreferencesWidget() = default;

    public slots:
        void applySettings();

    signals:
        void clearRefreshTokens();

    private:
        SimpleCrypt mCrypt;

        QRadioButton *mNoProxyBtn = nullptr;
        QComboBox *mProxyTypeCombo = nullptr;
        QLineEdit *mProxyHostEdit = nullptr;
        QSpinBox *mProxyPortEdit = nullptr;
        QLineEdit *mProxyUserEdit = nullptr;
        QLineEdit *mProxyPasswordEdit = nullptr;

        QSpinBox *mMaxReplyTimeEdit = nullptr;
        QSpinBox *mMaxRetriesEdit = nullptr;
        QCheckBox *mIgnoreSslErrors = nullptr;
        QCheckBox *mLogESIReplies = nullptr;
        QCheckBox *mUseHTTP2 = nullptr;
    };
}
