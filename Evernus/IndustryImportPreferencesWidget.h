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

class QCheckBox;

namespace Evernus
{
    class IndustryImportPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit IndustryImportPreferencesWidget(QWidget *parent = nullptr);
        IndustryImportPreferencesWidget(const IndustryImportPreferencesWidget &) = default;
        IndustryImportPreferencesWidget(IndustryImportPreferencesWidget &&) = default;
        virtual ~IndustryImportPreferencesWidget() = default;

        IndustryImportPreferencesWidget &operator =(const IndustryImportPreferencesWidget &) = default;
        IndustryImportPreferencesWidget &operator =(IndustryImportPreferencesWidget &&) = default;

    public slots:
        void applySettings();

    private:
        QCheckBox *mImportMiningLedger = nullptr;
    };
}
