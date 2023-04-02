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

#include "MarketHistory.h"

class QCPGraph;
class QCPAxis;

namespace Evernus
{
    class DateFilteredPlotWidget;

    class DoubleTypeCompareWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        DoubleTypeCompareWidget(MarketHistory firstHistory,
                                MarketHistory secondHistory,
                                const QString &firstInfo,
                                const QString &secondInfo,
                                QWidget *parent = nullptr);
        DoubleTypeCompareWidget(const DoubleTypeCompareWidget &) = default;
        DoubleTypeCompareWidget(DoubleTypeCompareWidget &&) = default;
        virtual ~DoubleTypeCompareWidget() = default;

        DoubleTypeCompareWidget &operator =(const DoubleTypeCompareWidget &) = default;
        DoubleTypeCompareWidget &operator =(DoubleTypeCompareWidget &&) = default;

    public slots:
        void handleNewPreferences();

    private slots:
        void applyFilter();

    private:
        MarketHistory mFirstHistory;
        MarketHistory mSecondHistory;

        DateFilteredPlotWidget *mHistoryPlot = nullptr;

        QCPGraph *mFirstPriceGraph = nullptr;
        QCPGraph *mSecondPriceGraph = nullptr;
        QCPGraph *mFirstVolumeGraph = nullptr;
        QCPGraph *mSecondVolumeGraph = nullptr;
        QCPAxis *mVolumeDateAxis = nullptr;

        void applyGraphFormats();
    };
}
