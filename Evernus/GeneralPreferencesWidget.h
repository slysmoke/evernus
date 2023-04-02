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

#include <QSqlDatabase>
#include <QWidget>

class QCheckBox;
class QLineEdit;
class QComboBox;

namespace Evernus
{
    class LanguageComboBox;

    class GeneralPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit GeneralPreferencesWidget(const QSqlDatabase &db, QWidget *parent = nullptr);
        virtual ~GeneralPreferencesWidget() = default;

    public slots:
        void applySettings();

    private:
        QSqlDatabase mDb;

        LanguageComboBox *mLanguageEdit = nullptr;
        QCheckBox *mMinimizeToTrayBtn = nullptr;
        QCheckBox *mMinimizeByMarginToolBtn = nullptr;
        QCheckBox *mAutoUpdateBtn = nullptr;
        QCheckBox *mUsePackagedVolumeBtn = nullptr;
        QCheckBox *mOmitCurrencySymbolBtn = nullptr;
        QCheckBox *mUseUTCDatesBtn = nullptr;
        QLineEdit *mDateFormEdit = nullptr;
        QCheckBox *mApplyDateFormatToGraphsBtn = nullptr;
        QComboBox *mColumnDelimiterEdit = nullptr;
        QComboBox *mDbSynchronousEdit = nullptr;
    };
}
