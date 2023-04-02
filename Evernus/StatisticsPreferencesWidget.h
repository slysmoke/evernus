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
class QComboBox;
class QSpinBox;

namespace Evernus
{
    class ColorButton;

    class StatisticsPreferencesWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        explicit StatisticsPreferencesWidget(QWidget *parent = nullptr);
        virtual ~StatisticsPreferencesWidget() = default;

    public slots:
        void applySettings();

    private slots:
        void resetPlotColors();

    private:
        QCheckBox *mCombineCorpAndCharPlotsBtn = nullptr;
        QCheckBox *mAutomaticSnapshotsBtn = nullptr;

        QComboBox *mPlotNumberFormatEdit = nullptr;
        ColorButton *mAssetPlotColorBtn = nullptr;
        ColorButton *mCorpAssetPlotColorBtn = nullptr;
        ColorButton *mWalletPlotColorBtn = nullptr;
        ColorButton *mCorpWalletPlotColorBtn = nullptr;
        ColorButton *mBuyOrdersPlotColorBtn = nullptr;
        ColorButton *mSellOrdersPlotColorBtn = nullptr;
        ColorButton *mTotalPlotColorBtn = nullptr;

        QSpinBox *mDefaultAggregatedGraphPeriodEdit = nullptr;

        void addPlotFormat(const QString &text, const QString &value, const QString &curValue);
    };
}
