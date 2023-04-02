/**
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more Graph.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <map>

#include <QWidget>

#include "MarketHistoryEntry.h"
#include "VolumeType.h"

class QCPFinancial;
class QCustomPlot;
class QCPItemLine;
class QCPGraph;
class QCPBars;
class QDate;

namespace Evernus
{
    class TypeAggregatedGraphWidget
        : public QWidget
    {
        Q_OBJECT

    public:
        using History = std::map<QDate, MarketHistoryEntry>;

        explicit TypeAggregatedGraphWidget(History history, QWidget *parent = nullptr, Qt::WindowFlags flags = 0);
        virtual ~TypeAggregatedGraphWidget() = default;

    public slots:
        void handleNewPreferences();
        void showLegend(bool flag);
        void applyFilter(const QDate &start,
                         const QDate &end,
                         int smaDays,
                         int macdFastDays,
                         int macdSlowDays,
                         int macdEmaDays,
                         VolumeType volumeType);
        void addTrendLine(const QDate &start, const QDate &end);

    private:
        History mHistory;

        QCustomPlot *mHistoryPlot = nullptr;

        QCPFinancial *mHistoryValuesGraph = nullptr;
        QCPBars *mHistoryVolumeGraph = nullptr;
        QCPBars *mHistoryVolumeFlagGraph = nullptr;
        QCPGraph *mSMAGraph = nullptr;
        QCPGraph *mRSIGraph = nullptr;
        QCPGraph *mMACDGraph = nullptr;
        QCPGraph *mMACDEMAGraph = nullptr;
        QCPBars *mMACDDivergenceGraph = nullptr;
        QCPGraph *mBollingerUpperGraph = nullptr;
        QCPGraph *mBollingerLowerGraph = nullptr;

        QCPItemLine *mTrendLine = nullptr;

        void deleteTrendLine() noexcept;
        void applyGraphFormats();
    };
}
