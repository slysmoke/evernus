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

class QComboBox;

namespace Evernus
{
    class ImportSourcePreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit ImportSourcePreferencesWidget(QWidget *parent = nullptr);
        virtual ~ImportSourcePreferencesWidget() = default;

    public slots:
        void applySettings();

    private:
        QComboBox *mPriceSourceCombo = nullptr;
        QComboBox *mMarketOrderSourceCombo = nullptr;
        QComboBox *mMarketOrderImportTypeCombo = nullptr;

        template<class T>
        void addSourceItem(QComboBox &combo, const QString &text, T value, T current);
    };
}
