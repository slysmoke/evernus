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

class QPushButton;
class QCheckBox;
class QSpinBox;

namespace Evernus
{
    class EveDataProvider;

    class OrderPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit OrderPreferencesWidget(const EveDataProvider &dataProvider, QWidget *parent = nullptr);
        virtual ~OrderPreferencesWidget() = default;

    public slots:
        void applySettings();

    private slots:
        void chooseDefaultCustomStation();

    private:
        const EveDataProvider &mDataProvider;

        QSpinBox *mMarketOrderMaxAgeEdit = nullptr;
        QCheckBox *mDeleteOldMarketOrdersBtn = nullptr;
        QSpinBox *mOldMarketOrdersDaysEdit = nullptr;
        QCheckBox *mLimitSellToStationBtn = nullptr;
        QSpinBox *mVolumeWarningEdit = nullptr;
        QPushButton *mDefaultCustomStationBtn = nullptr;
        QCheckBox *mImportFromCitadels = nullptr;

        QVariant mDefaultCustomStation;
    };
}
