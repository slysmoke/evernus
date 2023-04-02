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

#include <QString>

#include "SizeRememberingWidget.h"
#include "MarketHistory.h"

class QCPFinancial;
class QCustomPlot;
class QCPItemLine;
class QCPGraph;
class QCPBars;

namespace Evernus
{
    class TypeAggregatedDetailsFilterWidget;
    class TypeAggregatedGraphWidget;

    class DoubleTypeAggregatedDetailsWidget
        : public SizeRememberingWidget
    {
        Q_OBJECT

    public:
        DoubleTypeAggregatedDetailsWidget(MarketHistory firstHistory,
                                          MarketHistory secondHistory,
                                          const QString &firstInfo,
                                          const QString &secondInfo,
                                          QWidget *parent = nullptr,
                                          Qt::WindowFlags flags = 0);
        virtual ~DoubleTypeAggregatedDetailsWidget() = default;

    public slots:
        void handleNewPreferences();

    private:
        TypeAggregatedDetailsFilterWidget *mFilterWidget = nullptr;
        TypeAggregatedGraphWidget *mFirstGraphWidget = nullptr;
        TypeAggregatedGraphWidget *mSecondGraphWidget = nullptr;
    };
}
